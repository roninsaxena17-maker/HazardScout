
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

// =====================================================
// HAZARDSCOUT - ESP32-CAM
// Board: AI-Thinker ESP32-CAM
// Camera: OV2640
// =====================================================

// Wi-Fi credentials
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// =====================================================
// AI-THINKER CAMERA PIN CONFIGURATION
// =====================================================

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// =====================================================
// HTTP STREAM SERVER
// =====================================================

static httpd_handle_t stream_httpd = NULL;
static httpd_handle_t camera_httpd = NULL;

#define PART_BOUNDARY "hazardscoutboundary"

static const char* STREAM_CONTENT_TYPE =
  "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;

static const char* STREAM_BOUNDARY =
  "\r\n--" PART_BOUNDARY "\r\n";

static const char* STREAM_PART =
  "Content-Type: image/jpeg\r\n"
  "Content-Length: %u\r\n\r\n";

// =====================================================
// CAMERA STREAM HANDLER
// =====================================================

static esp_err_t stream_handler(httpd_req_t *req) {

  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;

  size_t jpg_buf_len = 0;
  uint8_t *jpg_buf = NULL;

  char part_buf[64];

  res = httpd_resp_set_type(
    req,
    STREAM_CONTENT_TYPE
  );

  if (res != ESP_OK) {
    return res;
  }

  httpd_resp_set_hdr(
    req,
    "Access-Control-Allow-Origin",
    "*"
  );

  while (true) {

    fb = esp_camera_fb_get();

    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    }

    if (res == ESP_OK) {

      if (fb->format != PIXFORMAT_JPEG) {

        bool converted = frame2jpg(
          fb,
          80,
          &jpg_buf,
          &jpg_buf_len
        );

        esp_camera_fb_return(fb);
        fb = NULL;

        if (!converted) {
          Serial.println("JPEG conversion failed");
          res = ESP_FAIL;
        }

      } else {

        jpg_buf_len = fb->len;
        jpg_buf = fb->buf;
      }
    }

    if (res == ESP_OK) {

      res = httpd_resp_send_chunk(
        req,
        STREAM_BOUNDARY,
        strlen(STREAM_BOUNDARY)
      );
    }

    if (res == ESP_OK) {

      size_t hlen = snprintf(
        part_buf,
        sizeof(part_buf),
        STREAM_PART,
        jpg_buf_len
      );

      res = httpd_resp_send_chunk(
        req,
        part_buf,
        hlen
      );
    }

    if (res == ESP_OK) {

      res = httpd_resp_send_chunk(
        req,
        (const char*)jpg_buf,
        jpg_buf_len
      );
    }

    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      jpg_buf = NULL;
    } else if (jpg_buf) {
      free(jpg_buf);
      jpg_buf = NULL;
    }

    if (res != ESP_OK) {
      break;
    }
  }

  return res;
}

// =====================================================
// SINGLE IMAGE CAPTURE HANDLER
// =====================================================

static esp_err_t capture_handler(httpd_req_t *req) {

  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");

  httpd_resp_set_hdr(
    req,
    "Access-Control-Allow-Origin",
    "*"
  );

  esp_err_t res = httpd_resp_send(
    req,
    (const char*)fb->buf,
    fb->len
  );

  esp_camera_fb_return(fb);

  return res;
}

// =====================================================
// CAMERA CONTROL PAGE
// =====================================================

static esp_err_t index_handler(httpd_req_t *req) {

  const char* html =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<title>HazardScout Camera</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "</head>"
    "<body style='background:#111;color:white;text-align:center;font-family:Arial'>"
    "<h2>HazardScout Live Camera</h2>"
    "<img src='/stream' style='width:95%;max-width:800px'>"
    "<br><br>"
    "<a href='/capture' style='color:white'>Capture Image</a>"
    "</body>"
    "</html>";

  httpd_resp_set_type(req, "text/html");

  return httpd_resp_send(
    req,
    html,
    HTTPD_RESP_USE_STRLEN
  );
}

// =====================================================
// START WEB SERVERS
// =====================================================

void startCameraServer() {

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  config.server_port = 80;
  config.ctrl_port = 32768;

  httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL
  };

  httpd_uri_t capture_uri = {
    .uri = "/capture",
    .method = HTTP_GET,
    .handler = capture_handler,
    .user_ctx = NULL
  };

  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {

    httpd_register_uri_handler(
      camera_httpd,
      &index_uri
    );

    httpd_register_uri_handler(
      camera_httpd,
      &capture_uri
    );
  }

  config.server_port = 81;
  config.ctrl_port = 32769;

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {

    httpd_register_uri_handler(
      stream_httpd,
      &stream_uri
    );
  }
}

// =====================================================
// CAMERA INITIALIZATION
// =====================================================

void setupCamera() {

  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;

  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {

    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

  } else {

    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;

  esp_err_t error = esp_camera_init(&config);

  if (error != ESP_OK) {

    Serial.printf(
      "Camera initialization failed: 0x%x\n",
      error
    );

    return;
  }

  Serial.println("Camera initialized successfully");
}

// =====================================================
// WIFI CONNECTION
// =====================================================

void connectWiFi() {

  WiFi.mode(WIFI_STA);

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();

  Serial.println("Wi-Fi connected");

  Serial.print("ESP32-CAM IP: ");

  Serial.println(WiFi.localIP());
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  Serial.println();
  Serial.println("HAZARDSCOUT CAMERA STARTING");

  connectWiFi();

  setupCamera();

  startCameraServer();

  Serial.println("Camera server started");

  Serial.print("Camera page: http://");
  Serial.println(WiFi.localIP());

  Serial.print("Video stream: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":81/stream");
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("Wi-Fi disconnected");

    WiFi.reconnect();
  }

  delay(1000);
}
