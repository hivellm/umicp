# Publish script for UMICP Kotlin SDK to Maven Central (PowerShell)
# Usage: .\publish.ps1

$ErrorActionPreference = "Stop"

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

Write-Host "Publishing UMICP Kotlin SDK to Maven Central..." -ForegroundColor Blue

# Check if Java is installed
if (-not (Get-Command java -ErrorAction SilentlyContinue)) {
    Write-Host "Java not found." -ForegroundColor Red
    exit 1
}

# Get version from build.gradle.kts
$BuildGradle = Get-Content build.gradle.kts
$Version = ($BuildGradle | Select-String -Pattern 'version = "(.*?)"').Matches.Groups[1].Value

Write-Host "Package: com.hivellm:umicp-kotlin"
Write-Host "Version: $Version"
Write-Host ""

# Check if artifacts exist
if (-not (Test-Path "build\libs") -or (Get-ChildItem "build\libs\*.jar" -ErrorAction SilentlyContinue).Count -eq 0) {
    Write-Host "No build artifacts found. Running build..." -ForegroundColor Yellow
    .\build.ps1
}

# Check for signing credentials
if (-not $env:ORG_GRADLE_PROJECT_signingKey -and -not $env:ORG_GRADLE_PROJECT_signingKeyId) {
    Write-Host "No signing credentials found!" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "For Maven Central, you need to configure signing."
    Write-Host ""
    Write-Host "Set environment variables:"
    Write-Host '  $env:ORG_GRADLE_PROJECT_signingKeyId="YOUR_KEY_ID"'
    Write-Host '  $env:ORG_GRADLE_PROJECT_signingKey="YOUR_GPG_KEY"'
    Write-Host '  $env:ORG_GRADLE_PROJECT_signingPassword="YOUR_PASSWORD"'
    Write-Host ""
    Write-Host '  $env:ORG_GRADLE_PROJECT_ossrhUsername="YOUR_SONATYPE_USERNAME"'
    Write-Host '  $env:ORG_GRADLE_PROJECT_ossrhPassword="YOUR_SONATYPE_PASSWORD"'
    Write-Host ""
    Write-Host "Or add to ~/.gradle/gradle.properties:"
    Write-Host "  signingKeyId=YOUR_KEY_ID"
    Write-Host "  signingKey=YOUR_GPG_KEY"
    Write-Host "  signingPassword=YOUR_PASSWORD"
    Write-Host "  ossrhUsername=YOUR_SONATYPE_USERNAME"
    Write-Host "  ossrhPassword=YOUR_SONATYPE_PASSWORD"
    Write-Host ""
    $localOnly = Read-Host "Continue with publish to local Maven? (yes/no)"
    if ($localOnly -ne "yes") {
        Write-Host "Publish cancelled."
        exit 0
    }

    # Publish to local Maven
    Write-Host "Publishing to local Maven repository..." -ForegroundColor Blue
    .\gradlew.bat publishToMavenLocal --no-daemon

    Write-Host ""
    Write-Host "Published to local Maven!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Package available at: $env:USERPROFILE\.m2\repository\com\hivellm\umicp-kotlin\$Version\"
    Write-Host ""
    Write-Host "Test installation:"
    Write-Host "  Add to build.gradle.kts:"
    Write-Host "    repositories { mavenLocal() }"
    Write-Host "    dependencies { implementation(`"com.hivellm:umicp-kotlin:$Version`") }"
    Write-Host ""
    exit 0
}

# Confirm publish (unless in CI)
if (-not $env:CI) {
    Write-Host "You are about to publish to Maven Central!" -ForegroundColor Yellow
    Write-Host ""
    $confirm = Read-Host "Continue with publish to Maven Central? (yes/no)"
    if ($confirm -ne "yes") {
        Write-Host "Publish cancelled."
        exit 0
    }
}

# Publish to Maven Central
Write-Host "Publishing to Maven Central..." -ForegroundColor Blue
.\gradlew.bat publish --no-daemon

# Success message
Write-Host ""
Write-Host "Package published successfully!" -ForegroundColor Green
Write-Host ""
Write-Host "View on Maven Central:"
Write-Host "  https://central.sonatype.com/artifact/com.hivellm/umicp-kotlin/$Version"
Write-Host ""
Write-Host "Installation:"
Write-Host "  implementation(`"com.hivellm:umicp-kotlin:$Version`")"
Write-Host ""
Write-Host "Note: It may take a few hours for the package to be available on Maven Central."
Write-Host ""

