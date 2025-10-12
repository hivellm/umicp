#!/bin/bash

DEPLOYMENT_ID="baf9aaae-9ede-461c-8bd9-d550e1bdbe0d"
USERNAME="opdfUz"
PASSWORD="CFMurYDz3s3Ul9xOLDINWXmE93lCBwa4v"

echo "🔍 Checking Maven Central deployment status..."
echo "Deployment ID: $DEPLOYMENT_ID"
echo ""

for i in {1..5}; do
    echo "Attempt $i/5..."

    RESPONSE=$(curl -s -X GET \
        "https://central.sonatype.com/api/v1/publisher/status?id=$DEPLOYMENT_ID" \
        -u "$USERNAME:$PASSWORD")

    echo "Response: $RESPONSE"
    echo ""

    if echo "$RESPONSE" | grep -q '"deploymentState":"PUBLISHED"'; then
        echo "✅ Successfully published!"
        echo ""
        echo "🎉 Your package is now available at:"
        echo "   https://central.sonatype.com/artifact/com.hivellm.org/umicp-kotlin/0.1.3"
        echo ""
        echo "📦 Maven Central (may take a few hours to sync):"
        echo "   https://repo1.maven.org/maven2/com/hivellm/org/umicp-kotlin/0.1.3/"
        exit 0
    elif echo "$RESPONSE" | grep -q '"deploymentState":"FAILED"'; then
        echo "❌ Deployment failed!"
        echo "$RESPONSE" | jq . || echo "$RESPONSE"
        exit 1
    elif echo "$RESPONSE" | grep -q '"deploymentState":"VALIDATING"'; then
        echo "⏳ Still validating..."
    elif echo "$RESPONSE" | grep -q '"deploymentState":"VALIDATED"'; then
        echo "✅ Validated, publishing..."
    elif echo "$RESPONSE" | grep -q '"deploymentState":"PENDING"'; then
        echo "⏳ Pending..."
    else
        echo "⚠️  Unknown state or error"
    fi

    if [ $i -lt 5 ]; then
        echo "Waiting 10 seconds..."
        sleep 10
    fi
done

echo ""
echo "📋 Check status manually at:"
echo "   https://central.sonatype.com/publishing/deployments"
echo ""
echo "Deployment ID: $DEPLOYMENT_ID"

