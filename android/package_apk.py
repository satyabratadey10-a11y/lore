#!/usr/bin/env python3
"""
package_apk.py - Gradle-free APK packager for LORE NativeActivity applications.
Steps:
1. Prepares AndroidManifest.xml from metadata.
2. Uses aapt2 link to produce base APK containing compiled binary manifest and assets.
3. Injects compiled shared libraries into lib/<abi>/lib<name>.so.
4. Zipaligns the archive (4-byte alignment).
5. Signs the APK with apksigner using a debug keystore.
"""

import argparse
import glob
import os
import shutil
import subprocess
import sys
import zipfile


def find_sdk_tool(tool_name, sdk_root=None):
    # 1. Search PATH
    path_tool = shutil.which(tool_name)
    if path_tool:
        return path_tool

    # Candidate SDK directories
    candidates = []
    if sdk_root:
        candidates.append(sdk_root)
    if os.environ.get("ANDROID_HOME"):
        candidates.append(os.environ.get("ANDROID_HOME"))
    if os.environ.get("ANDROID_SDK_ROOT"):
        candidates.append(os.environ.get("ANDROID_SDK_ROOT"))
    candidates.extend([
        "/usr/local/lib/android/sdk",
        "/opt/android-sdk",
        os.path.expanduser("~/Android/Sdk"),
        "/sdcard/Download/toolchain/opt/android-sdk"
    ])

    for base in candidates:
        if not base or not os.path.exists(base):
            continue
        # Check build-tools
        build_tools_dirs = sorted(glob.glob(os.path.join(base, "build-tools", "*")), reverse=True)
        for btd in build_tools_dirs:
            candidate = os.path.join(btd, tool_name)
            if os.path.isfile(candidate) and os.access(candidate, os.X_OK):
                return candidate
    return None


def find_android_jar(sdk_root=None):
    candidates = []
    if sdk_root:
        candidates.append(sdk_root)
    if os.environ.get("ANDROID_HOME"):
        candidates.append(os.environ.get("ANDROID_HOME"))
    if os.environ.get("ANDROID_SDK_ROOT"):
        candidates.append(os.environ.get("ANDROID_SDK_ROOT"))
    candidates.extend([
        "/usr/local/lib/android/sdk",
        "/opt/android-sdk",
        os.path.expanduser("~/Android/Sdk"),
        "/sdcard/Download/toolchain/opt/android-sdk"
    ])

    for base in candidates:
        if not base or not os.path.exists(base):
            continue
        platforms = sorted(glob.glob(os.path.join(base, "platforms", "android-*")), reverse=True)
        for plat in platforms:
            jar = os.path.join(plat, "android.jar")
            if os.path.isfile(jar):
                return jar
    return None


def ensure_debug_keystore(keystore_path):
    if os.path.exists(keystore_path):
        return
    os.makedirs(os.path.dirname(os.path.abspath(keystore_path)), exist_ok=True)
    keytool = shutil.which("keytool")
    if not keytool:
        print("[WARN] keytool not found; cannot create keystore automatically.", file=sys.stderr)
        return
    cmd = [
        keytool, "-genkey", "-v",
        "-keystore", keystore_path,
        "-storepass", "android",
        "-alias", "androiddebugkey",
        "-keypass", "android",
        "-keyalg", "RSA",
        "-keysize", "2048",
        "-validity", "10000",
        "-dname", "CN=Android Debug,O=Android,C=US"
    ]
    subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"[INFO] Created debug keystore at {keystore_path}")


def main():
    parser = argparse.ArgumentParser(description="Package LORE NativeActivity APK without Gradle.")
    parser.add_argument("--manifest", help="Path to AndroidManifest.xml")
    parser.add_argument("--package-name", default="com.axilo.app", help="Package ID")
    parser.add_argument("--package-label", default="LORE App", help="App display label")
    parser.add_argument("--lib-name", default="app", help="Native library name without lib and .so")
    parser.add_argument("--min-sdk", default="26", help="minSdkVersion")
    parser.add_argument("--target-sdk", default="33", help="targetSdkVersion")
    parser.add_argument("--so", required=True, help="Path to compiled .so")
    parser.add_argument("--abi", default="arm64-v8a", help="Target ABI (default: arm64-v8a)")
    parser.add_argument("--assets", help="Path to assets folder")
    parser.add_argument("--out", required=True, help="Output APK path")
    parser.add_argument("--sdk-root", help="Android SDK root directory")
    parser.add_argument("--keystore", help="Path to debug keystore")
    parser.add_argument("--dry-run", action="store_true", help="Prepare files only")

    args = parser.parse_args()

    out_apk = os.path.abspath(args.out)
    os.makedirs(os.path.dirname(out_apk), exist_ok=True)
    work_dir = os.path.join(os.path.dirname(out_apk), "_apk_build_tmp")
    shutil.rmtree(work_dir, ignore_errors=True)
    os.makedirs(work_dir, exist_ok=True)

    manifest_path = args.manifest
    if not manifest_path or not os.path.exists(manifest_path):
        manifest_path = os.path.join(work_dir, "AndroidManifest.xml")
        template_content = f"""<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="{args.package_name}"
    android:versionCode="1"
    android:versionName="1.0">

    <uses-sdk android:minSdkVersion="{args.min_sdk}" android:targetSdkVersion="{args.target_sdk}" />

    <application
        android:label="{args.package_label}"
        android:hasCode="false">
        <activity
            android:name="android.app.NativeActivity"
            android:label="{args.package_label}"
            android:configChanges="orientation|keyboardHidden|screenSize"
            android:exported="true">
            <meta-data
                android:name="android.app.lib_name"
                android:value="{args.lib_name}" />
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
"""
        with open(manifest_path, "w", encoding="utf-8") as f:
            f.write(template_content)

    print(f"[INFO] Using manifest: {manifest_path}")

    aapt2 = find_sdk_tool("aapt2", args.sdk_root)
    zipalign = find_sdk_tool("zipalign", args.sdk_root)
    apksigner = find_sdk_tool("apksigner", args.sdk_root)
    android_jar = find_android_jar(args.sdk_root)

    print(f"[INFO] aapt2: {aapt2}")
    print(f"[INFO] zipalign: {zipalign}")
    print(f"[INFO] apksigner: {apksigner}")
    print(f"[INFO] android.jar: {android_jar}")

    if not aapt2 or not android_jar:
        print("[ERROR] aapt2 and android.jar are required to package an Android APK.", file=sys.stderr)
        print("Please ensure Android SDK build-tools and platforms are installed.", file=sys.stderr)
        sys.exit(1)

    # Step 1: aapt2 link
    base_apk = os.path.join(work_dir, "base.apk")
    aapt_cmd = [
        aapt2, "link",
        "-I", android_jar,
        "--manifest", manifest_path,
        "-o", base_apk
    ]
    if args.assets and os.path.isdir(args.assets):
        aapt_cmd.extend(["-A", args.assets])

    print(f"[INFO] Running aapt2 link: {' '.join(aapt_cmd)}")
    subprocess.run(aapt_cmd, check=True)

    # Step 2: Inject shared library into base.apk
    # Format inside APK: lib/<abi>/lib<lib_name>.so
    lib_filename = f"lib{args.lib_name}.so" if not os.path.basename(args.so).startswith("lib") else os.path.basename(args.so)
    apk_lib_path = f"lib/{args.abi}/{lib_filename}"

    with zipfile.ZipFile(base_apk, "a") as z:
        print(f"[INFO] Adding native library {args.so} as {apk_lib_path}")
        z.write(args.so, apk_lib_path)
        # If assets provided and not already bundled by aapt2, add them
        if args.assets and os.path.isdir(args.assets):
            for root, _, files in os.walk(args.assets):
                for file in files:
                    full_p = os.path.join(root, file)
                    rel_p = os.path.relpath(full_p, args.assets)
                    asset_zip_p = f"assets/{rel_p}"
                    if asset_zip_p not in z.namelist():
                        z.write(full_p, asset_zip_p)

    # Step 3: Zipalign
    unaligned_apk = base_apk
    aligned_apk = os.path.join(work_dir, "aligned.apk")
    if zipalign:
        print(f"[INFO] Zipaligning APK (4-byte alignment)...")
        zipalign_cmd = [zipalign, "-f", "-p", "4", unaligned_apk, aligned_apk]
        subprocess.run(zipalign_cmd, check=True)
    else:
        print("[WARN] zipalign not found; using unaligned archive.")
        aligned_apk = unaligned_apk

    # Step 4: Sign APK
    keystore_path = args.keystore or os.path.join(os.path.dirname(os.path.abspath(__file__)), "debug.keystore")
    ensure_debug_keystore(keystore_path)

    if apksigner and os.path.exists(keystore_path):
        print(f"[INFO] Signing APK with apksigner...")
        sign_cmd = [
            apksigner, "sign",
            "--ks", keystore_path,
            "--ks-pass", "pass:android",
            "--key-pass", "pass:android",
            "--out", out_apk,
            aligned_apk
        ]
        subprocess.run(sign_cmd, check=True)
        print(f"[SUCCESS] Signed APK generated at: {out_apk}")
    else:
        # If apksigner is not available, copy aligned APK to out_apk
        shutil.copyfile(aligned_apk, out_apk)
        print(f"[WARN] apksigner not available or keystore missing. Output APK at: {out_apk}")

    # Cleanup temp dir
    shutil.rmtree(work_dir, ignore_errors=True)


if __name__ == "__main__":
    main()
