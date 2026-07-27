import os
import json
import shutil
import argparse

def main():
    parser = argparse.ArgumentParser(description="Package firmware for ESP Web Tools")
    parser.add_argument("--build-dir", default="build", help="Path to the build directory")
    parser.add_argument("--output-dir", default="web_install", help="Path to the output directory")
    args = parser.parse_args()

    build_dir = args.build_dir
    output_dir = args.output_dir

    flasher_args_path = os.path.join(build_dir, "flasher_args.json")
    if not os.path.exists(flasher_args_path):
        print(f"Error: {flasher_args_path} not found. Please build the project first.")
        return

    with open(flasher_args_path, "r") as f:
        flasher_args = json.load(f)

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    chip = flasher_args.get("extra_esptool_args", {}).get("chip", "esp32s3").upper()

    parts = []
    flash_files = flasher_args.get("flash_files", {})
    
    print(f"Packaging files for {chip} into {output_dir}/...")

    for offset_str, file_path in flash_files.items():
        src_path = os.path.join(build_dir, file_path)
        if not os.path.exists(src_path):
            print(f"Warning: {src_path} not found, skipping.")
            continue

        filename = os.path.basename(file_path)
        dest_path = os.path.join(output_dir, filename)
        shutil.copy2(src_path, dest_path)
        print(f"Copied {filename} (offset: {offset_str})")

        # Convert offset string (like "0x1000") to integer
        offset = int(offset_str, 16)
        
        parts.append({
            "path": filename,
            "offset": offset
        })

    manifest = {
        "name": "XiaoZhi",
        "version": "latest",
        "builds": [
            {
                "chipFamily": chip,
                "parts": parts
            }
        ]
    }

    manifest_path = os.path.join(output_dir, "manifest.json")
    with open(manifest_path, "w") as f:
        json.dump(manifest, f, indent=2)
    print(f"Generated {manifest_path}")

    html_content = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Install XiaoZhi</title>
    <style>
        body {
            font-family: -apple-system, system-ui, BlinkMacSystemFont, "Segoe UI", Roboto, Ubuntu;
            padding: 2rem;
            max-width: 600px;
            margin: 0 auto;
            text-align: center;
        }
        .container {
            background-color: #f8f9fa;
            border-radius: 8px;
            padding: 2rem;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        .qr-section {
            margin-top: 2rem;
            padding-top: 2rem;
            border-top: 1px solid #dee2e6;
        }
        .qr-section img {
            max-width: 150px;
            border-radius: 4px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Install XiaoZhi Firmware</h1>
        <p>Connect your ESP32 device to your computer via USB and click the button below to install the firmware.</p>
        <p><em>Note: This requires a Web Serial API compatible browser (like Chrome or Edge).</em></p>
        <script type="module" src="https://unpkg.com/esp-web-tools@10/dist/web/install-button.js?module"></script>
        <esp-web-install-button manifest="manifest.json"></esp-web-install-button>
        
        <div class="qr-section">
            <h3>Wi-Fi Configuration</h3>
            <p>Scan this QR code with your phone to quickly connect to the Wi-Fi network:</p>
            <!-- QR code generated for WIFI:S:MatTroiNho-3D5D;T:WPA;P:;; -->
            <img src="https://api.qrserver.com/v1/create-qr-code/?size=150x150&data=WIFI%3AS%3AMatTroiNho-3D5D%3BT%3AWPA%3BP%3A%3B%3B" alt="Wi-Fi QR Code for MatTroiNho-3D5D" />
            <p><strong>SSID:</strong> MatTroiNho-3D5D</p>
        </div>
    </div>
</body>
</html>
"""
    
    html_path = os.path.join(output_dir, "index.html")
    with open(html_path, "w") as f:
        f.write(html_content)
    print(f"Generated {html_path}")
    
    print("\\nPackage complete!")
    print(f"To test, run a local web server in the '{output_dir}' directory:")
    print(f"  cd {output_dir}")
    print(f"  python3 -m http.server 8080")
    print("Then open http://localhost:8080 in Chrome.")

if __name__ == "__main__":
    main()
