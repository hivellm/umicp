#!/bin/bash

set -e

BUNDLE_DIR="maven-bundle/com/hivellm/org/umicp-kotlin/0.1.3"

echo "🔐 Generating checksums and signatures..."

cd "$BUNDLE_DIR"

# For each file (excluding checksums and signatures)
for file in *.jar *.pom *.module; do
    if [ -f "$file" ]; then
        echo "Processing: $file"

        # Generate MD5
        md5sum "$file" | cut -d' ' -f1 > "$file.md5"

        # Generate SHA1
        sha1sum "$file" | cut -d' ' -f1 > "$file.sha1"

        # Generate SHA256
        sha256sum "$file" | cut -d' ' -f1 > "$file.sha256"

        # Generate SHA512
        sha512sum "$file" | cut -d' ' -f1 > "$file.sha512"

        # Sign with GPG
        if command -v gpg &> /dev/null; then
            export GPG_TTY=$(tty)
            echo "test" | gpg --pinentry-mode loopback --batch --yes --passphrase-fd 0 --armor --detach-sign "$file" 2>/dev/null || \
            gpg --batch --yes --armor --detach-sign "$file" 2>/dev/null || \
            echo "  ⚠️  GPG signing failed for $file (will try alternative method)"

            if [ -f "$file.asc" ]; then
                echo "  ✅ Signed: $file.asc"
            fi
        else
            echo "  ⚠️  GPG not found, skipping signature for $file"
        fi

        echo "  ✅ MD5: $file.md5"
        echo "  ✅ SHA1: $file.sha1"
        echo "  ✅ SHA256: $file.sha256"
        echo "  ✅ SHA512: $file.sha512"
    fi
done

echo ""
echo "📦 Files generated:"
ls -lh

cd /mnt/f/Node/hivellm/umicp/bindings/kotlin

# Create bundle ZIP with correct structure
echo ""
echo "📦 Creating bundle ZIP..."
cd maven-bundle
zip -r ../umicp-kotlin-0.1.3-complete-bundle.zip com/
cd ..

echo ""
echo "✅ Complete bundle created: umicp-kotlin-0.1.3-complete-bundle.zip"
echo ""
echo "📊 Bundle contents:"
unzip -l umicp-kotlin-0.1.3-complete-bundle.zip

echo ""
echo "🎉 Done! Upload this bundle to Maven Central Portal"

