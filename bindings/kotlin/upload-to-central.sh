#!/bin/bash
# Upload UMICP Kotlin SDK to Maven Central Portal
# Uses the new Central Portal API

set -e

BUNDLE_NAME="umicp-kotlin-0.1.3-bundle.zip"
TOKEN="csbk8dyyw4"
API_URL="https://central.sonatype.com/api/v1/publisher/upload"

echo "📤 Uploading $BUNDLE_NAME to Maven Central..."

if [ ! -f "$BUNDLE_NAME" ]; then
    echo "❌ Bundle not found: $BUNDLE_NAME"
    echo "Run ./build.sh first to generate artifacts"
    exit 1
fi

# Upload via API
curl -X POST \
  "$API_URL?publishingType=AUTOMATIC" \
  -H "Authorization: Bearer $TOKEN" \
  -F "bundle=@$BUNDLE_NAME" \
  -v

echo ""
echo "✅ Upload completed!"
echo ""
echo "Check status at: https://central.sonatype.com/publishing"
echo "Package will be available at: https://central.sonatype.com/artifact/com.hivellm.org/umicp-kotlin/0.1.3"
echo ""

