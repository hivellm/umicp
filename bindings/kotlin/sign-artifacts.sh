#!/bin/bash

set -e

BUNDLE_DIR="maven-bundle/com/hivellm/org/umicp-kotlin/0.1.3"
GPG_KEY="D34A372C6F724FE4"

echo "🔐 Signing artifacts with GPG key: $GPG_KEY..."

# Configure GPG for non-interactive signing
export GPG_TTY=$(tty)
gpg-connect-agent updatestartuptty /bye >/dev/null 2>&1 || true

cd "$BUNDLE_DIR"

# Sign each artifact file
for file in *.jar *.pom *.module; do
    if [ -f "$file" ]; then
        echo "Signing: $file"
        gpg --pinentry-mode loopback --passphrase "" --batch --yes --local-user "$GPG_KEY" --armor --detach-sign "$file"

        if [ -f "$file.asc" ]; then
            echo "  ✅ Created signature: $file.asc"
        else
            echo "  ❌ Failed to sign: $file"
            exit 1
        fi
    fi
done

echo ""
echo "📦 All artifacts signed:"
ls -lh *.asc

cd /mnt/f/Node/hivellm/umicp/bindings/kotlin

# Recreate bundle with signatures
echo ""
echo "📦 Recreating bundle with signatures..."
rm -f umicp-kotlin-0.1.3-complete-bundle.zip
cd maven-bundle
zip -r ../umicp-kotlin-0.1.3-complete-bundle.zip com/
cd ..

echo ""
echo "✅ Complete bundle with signatures created!"
echo ""
echo "📊 Final bundle contents:"
unzip -l umicp-kotlin-0.1.3-complete-bundle.zip | head -50

