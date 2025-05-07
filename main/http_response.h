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