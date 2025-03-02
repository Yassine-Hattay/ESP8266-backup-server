#include "server_m.h"
#include "wifi_and_FS.h"
#include "esp_now_m.h"

char date[13] = "";
bool end_server_b = false;

void handleDatePhotos(AsyncWebServerRequest *request)
{

  lastTime2 = millis();

  char url[20];

  LittleFS.format();

  strcpy(url, request->url().c_str());

  char *firstSlash = strchr(url + 1, '/'); // Find the first slash after the initial '/'

  if (firstSlash != nullptr && strlen(date) == 0)
  {
    firstSlash++;             // Move past the initial slash
    strcpy(date, firstSlash); // Copy the substring into date

    // Remove trailing slash if present
    size_t len = strlen(date);
    while (len > 0 && date[len - 1] == '/')
    {
      date[len - 1] = '\0'; // Remove the last character (slash)
      len--;                // Decrease length to check the next character
    }

    // Remove leading slashes if present
    while (date[0] == '/')
    {
      // Shift all characters to the left by one (overwrite the first character)
      memmove(date, date + 1, strlen(date));
    }
  }

  char folderPath[21];
  snprintf(folderPath, sizeof(folderPath), "/photos/%s", date);

  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->print("<!DOCTYPE html><html><head><title>Photos from ");
  response->print(date);
  response->print("</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><style>");
  response->print("body{font-family:sans-serif;background:#f4f4f9;margin:0;padding:0;color:#333}h1{text-align:center;margin-top:20px;font-size:2.5rem;color:#333}p{text-align:center;margin:20px;color:#666}.gallery{display:flex;flex-wrap:wrap;justify-content:center;gap:20px;padding:20px}.gallery div{width:220px;height:auto;text-align:center;transition:transform 0.3s ease}.gallery div:hover{transform:scale(1.05)}.gallery img{width:100%;height:auto;border-radius:8px;object-fit:cover}.file-name{margin-top:10px;font-size:1rem;color:#333;padding:0 10px}.button{padding:12px 24px;font-size:1.2rem;border:none;border-radius:8px;background:#007BFF;color:white;cursor:pointer;margin-top:10px}.button:hover{background:#0056b3}.home-button{background:#28a745;position:fixed;top:20px;right:20px;border:none;border-radius:8px;padding:12px 24px;color:white;cursor:pointer;box-shadow:0 4px 8px rgba(0,0,0,0.1)}.home-button:hover{background:#218838}.home-button:disabled{background:gray;cursor:not-allowed}</style>");
  response->print("<script>");
  response->print("document.addEventListener('DOMContentLoaded', () => {");
  response->print("  const button = document.querySelector('.home-button');");
  response->print("  button.disabled = true;"); // Initially disable the button
  response->print("  setTimeout(() => {");
  response->print("    button.disabled = false;"); // Enable the button after 5 seconds
  response->print("  }, 5000);");
  response->print("});");
  response->print("function goHome() {");
  response->print("  const button = document.querySelector('.home-button');");
  response->print("  button.disabled = true;");          // Disable the button to prevent further clicks
  response->print("  button.innerText = 'Loading...';"); // Optionally update button text
  response->print("  fetch('/').then(r => r.text())");
  response->print("    .then(d => {");
  response->print("      document.body.innerHTML = d;"); // Replace the body content
  response->print("      window.location.href = '/';");  // Redirect the user to the root URL
  response->print("    })");
  response->print("    .catch(e => {");
  response->print("      console.error(e);");
  response->print("      button.disabled = false;");      // Re-enable button on error
  response->print("      button.innerText = 'Go Home';"); // Reset button text
  response->print("    });");
  response->print("}");
  response->print("</script></head><body>");
  response->print("<button class=\"home-button\" onclick=\"goHome()\">Go Home</button>");
  response->print("<h1>Photos from ");
  response->print(date);
  response->print("</h1><div class=\"gallery\">");

  if (!moreFiles)
  {
    root = SD.open(folderPath); // Open the directory for scanning
  }

  if (root && root.isDirectory())
  { uint8_t counter = 0;

    if (!moreFiles)
    {
      dir = root.openNextFile();
    }
    moreFiles = false;

    while (dir)
    {
      if (!dir.isDirectory())
      {
        char filePath[100]; // Ensure filePath is large enough

        const char *dirName = dir.name(); // Example directory name

        // Clear tempPath and filePath before using them
        memset(filePath, 0, sizeof(filePath));

        // Concatenate folderPath and dirName into tempPath
        snprintf(filePath, sizeof(filePath), "%s/%s", folderPath, dirName);

        // Check if the file ends with ".jpg"
        if (strlen(filePath) > 4 && strcmp(&filePath[strlen(filePath) - 4], ".jpg") == 0)
        {
          counter++;

          char littlefsPath[23];
          snprintf(littlefsPath, sizeof(littlefsPath), "/moon_littlefs_%d.jpg", counter);

          // Extract the file name without the extension
          char fileName[100]; // Ensure fileName is large enough
          // Copy the directory name to fileName
          strncpy(fileName, dirName, sizeof(fileName) - 1);

          // Find the last occurrence of the period (.) character
          char *dotPosition = strrchr(fileName, '.');

          // If a dot is found and it's not the last character
          if (dotPosition != NULL)
          {
            *dotPosition = '\0'; // Terminate the string at the dot
          }
          // Check LittleFS space
          LittleFS.info(fs_info);
          unsigned long freeSpace = fs_info.totalBytes - fs_info.usedBytes;
          if (freeSpace > 300000 && counter < 7)
          {
            // Only load the file if there's enough space
            loadFileToLittleFS(filePath, littlefsPath);
            for (int i = 0; fileName[i] != '\0'; i++)
            {
              if (fileName[i] == '-')
              {
                fileName[i] = ':'; // Replace '-' with ':'
              }
            }

            response->print("<div>");
            response->print("<div class=\"file-name\">");
            response->print(fileName); // Display file name without extension
            response->print("</div>");
            response->print("<a href=\"");
            response->print(littlefsPath);
            response->print("\" target=\"_blank\">");
            response->print("<img src=\"");
            response->print(littlefsPath);
            response->print("\" alt=\"");
            response->print(fileName);
            response->print("\"/></a>");
            response->print("</div>");
          }

          else
          {
            moreFiles = true;
            break; // Stop if there's not enough space for another file
          }
        }
      }
      dir = root.openNextFile();
    }
  }

  response->print("</div>");

  if (moreFiles)
  {
    response->print("<div style=\"text-align: center; margin-top: 20px;\">");
    response->print("<p id=\"countdown\" style=\"font-size: 1.2rem; color: #555;\">Wait to download more photos in 5</p>");
    response->print("<button id=\"loadButton\" onclick=\"loadMorePhotos()\" disabled style=\"font-size: 1.2rem; padding: 12px 24px; border: none; border-radius: 10px; background-color: #28a745; color: white; cursor: not-allowed; transition: all 0.3s ease;\">Load More Photos</button>");
    response->print("</div>");
    response->print("<script>");
    response->print("let countdown = 5;");
    response->print("let countdownElement = document.getElementById('countdown');");
    response->print("let loadButton = document.getElementById('loadButton');");
    response->print("let interval = setInterval(function() {");
    response->print("  countdown--;");
    response->print("  countdownElement.textContent = 'Wait to download more photos in ' + countdown;");
    response->print("  if (countdown <= 0) {");
    response->print("    clearInterval(interval);");
    response->print("    countdownElement.textContent = 'You can now load more photos!';");
    response->print("    loadButton.disabled = false;");
    response->print("    loadButton.style.cursor = 'pointer';"); // Changes cursor to pointer when button is enabled
    response->print("  }");
    response->print("}, 1000);");

    response->print("function loadMorePhotos(){");
    response->print("  loadButton.disabled = true;");              // Disable the button after it's clicked
    response->print("  loadButton.style.cursor = 'not-allowed';"); // Change cursor to not-allowed
    response->print("  loadButton.textContent = 'Loading...';");   // Update button text immediately

    // Redirect to /more immediately after button click
    response->print("  window.location.href = '/more';"); // Redirect to '/more'

    response->print("}");
    response->print("</script>");
  }

  response->print("</body></html>");
  request->send(response);
  printMemoryAndFileSystemStats();
}



void handleHome(AsyncWebServerRequest *request)
{

  lastTime2 = millis();

  LittleFS.format();
  moreFiles = false;
  date[0] = '\0';
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server", "ESP Async Web Server");

  // Start HTML document
  response->print(R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
          <title>Select Date</title>
          <meta charset="UTF-8">
          <meta name="viewport" content="width=device-width, initial-scale=1.0">
          <style>
              body {
                  font-family: 'Arial', sans-serif;
                  background-color: #f4f4f9;
                  color: #333;
                  margin: 0;
                  padding: 0;
                  display: flex;
                  flex-direction: column;
                  align-items: center;
                  justify-content: center;
                  height: 100vh;
              }
              h1 {
                  font-size: 2em;
                  margin-bottom: 20px;
                  color: #444;
                  text-align: center;
              }
              .container {
                  width: 90%;
                  max-width: 600px;
                  background: white;
                  border-radius: 8px;
                  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
                  padding: 20px;
                  display: flex;
                  flex-direction: column;
                  align-items: center;
              }
              .date-list {
                  list-style: none;
                  padding: 0;
                  margin: 0;
                  text-align: center;
              }
              .date-list li {
                  margin: 10px 0;
              }
              .date-list a {
                  text-decoration: none;
                  color: #007BFF;
                  font-weight: bold;
                  transition: color 0.3s ease;
              }
              .date-list a:hover {
                  color: #0056b3;
              }
              .disabled {
                  pointer-events: none;
                  color: gray;
                  cursor: not-allowed;
              }
              .top-right-button {
                  position: absolute;
                  top: 10px;
                  right: 10px;
              }
              .top-right-button button {
                  padding: 10px 20px;
                  font-size: 16px;
                  color: white;
                  background-color: #e74c3c;
                  border: none;
                  border-radius: 5px;
                  cursor: pointer;
                  transition: background-color 0.3s ease;
              }
              .top-right-button button:disabled {
                  background-color: gray;
                  cursor: not-allowed;
              }
              .top-right-button button:hover {
                  background-color: #c0392b;
              }
          </style>
          <script>
              function disableLinks(date) {
                  var links = document.querySelectorAll('.date-list a');
                  links.forEach(function(link) {
                      link.classList.add('disabled');
                      link.removeAttribute('href');
                  });
                  document.body.style.cursor = 'not-allowed';
                  window.location.href = '/photos/' + date;
              }
  
              function disableButton() {
                  document.documentElement.innerHTML = "<div style='display:flex; justify-content:center; align-items:center; height:100vh; font-size:10vw; font-weight:bold; color:red; background-color:black;'>x_x</div>";
                  setTimeout(function() {
                      window.location.href = '/server_off';
                  }, 500); // Optional delay before redirect
              }
          </script>
      </head>
      <body>
          <div class="top-right-button">
              <button onclick="disableButton();">
                  Turn Off Server
              </button>
          </div>
          <div class="container">
              <h1>Available Dates for Photos</h1>
              <ul class="date-list">
  )rawliteral");

  // Get the available dates by scanning SD card for folders
  File root_l = SD.open("/photos"); // Open the directory for scanning
  if (root_l && root_l.isDirectory())
  {
    File dir_l = root_l.openNextFile();
    while (dir_l)
    { // Iterate through the directory
      if (dir_l.isDirectory())
      {
        String date = dir_l.name();
        response->printf("<li><a href=\"#\" onclick=\"disableLinks('%s')\">%s</a></li>", date.c_str(), date.c_str());
      }
      dir_l = root_l.openNextFile();
    }
    dir_l.close();
  }

  root_l.close();
  dir.close();
  root.close();
  // Close HTML document
  response->print(R"rawliteral(
            </ul>
        </div>
    </body>
    </html>
    )rawliteral");

  request->send(response);
}



void server_end(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("text/plain"); // Set content type to plain text
  response->addHeader("Server", "ESP Async Web Server");

  // Send a simple OK response
  response->print("OK");

  request->send(response); // Send the response

  server.end();                     // Stop the server
  WiFi.disconnect();                // Disconnect WiFi
  WiFi.mode(WIFI_STA);              // Set WiFi mode to Station
  wifi_set_channel(ESPNOW_CHANNEL); // Set the WiFi channel

  WiFi.forceSleepBegin();
  delay(1); // Allow some time for WiFi to go into sleep

  // Set ESP8266 to light sleep
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T); // Set to light sleep mode
  wifi_fpm_open();                        // Open light sleep mod

  end_server_b = true;
}

void setup_server()
{
  uint8_t counter = 0;
  while (counter < 6)
  {
    counter++;

    char littlefsPath[23];
    snprintf(littlefsPath, sizeof(littlefsPath), "/moon_littlefs_%d.jpg", counter);

    server.on(littlefsPath, HTTP_GET, [littlefsPath](AsyncWebServerRequest *req)
              { req->send(LittleFS, littlefsPath, "image/jpeg"); });
  }

  server.on("/", HTTP_GET, handleHome);               // Show home page
  server.on("/photos/*", HTTP_GET, handleDatePhotos); // Serve photo file
  server.on("/more", HTTP_GET, handleDatePhotos);     // Serve photo file
  server.on("/server_off", HTTP_GET, server_end);     // Serve photo file

  server.begin();
  return;
}