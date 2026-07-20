# Publishing & CI — UMICP bindings

CI (lint + test) and release automation for the bindings that live in this repo.
`go` and `php` are maintained in their own repositories (`umicp-go`, `umicp-php`)
and carry their own CI.

## Bindings in this repo

| Binding    | Package                 | Registry      | Publish auth        | CI workflow          | Publish workflow       |
| ---------- | ----------------------- | ------------- | ------------------- | -------------------- | ---------------------- |
| python     | `umicp-sdk`             | PyPI          | OIDC (trusted)      | `ci-python.yml`      | `publish-python.yml`   |
| typescript | `@hivehub/umicp-sdk`    | npm           | OIDC (trusted)      | `ci-typescript.yml`  | `publish-npm.yml`      |
| rust       | `umicp-sdk`             | crates.io     | OIDC (trusted)      | `ci-rust.yml`        | `publish-rust.yml`     |
| csharp     | `HiveLLM.Umicp.SDK`     | NuGet.org     | OIDC (trusted)      | `ci-csharp.yml`      | `publish-nuget.yml`    |
| java       | `com.hivellm:umicp-sdk` | Maven Central | token + GPG         | `ci-java.yml`        | `publish-maven.yml`    |
| kotlin     | `com.hivellm.org:*`     | Maven Central | token + GPG         | `ci-kotlin.yml`      | `publish-maven.yml`    |
| elixir     | `umicp`                 | Hex.pm        | API key             | `ci-elixir.yml`      | `publish-hex.yml`      |
| swift      | `UMICP-SDK`             | SwiftPM (tag) | none (git tag)      | `ci-swift.yml`       | `publish-swift.yml`    |

## How a release runs

Publishing is driven by **GitHub Releases**. Create a release whose **tag** uses
the binding prefix; the matching workflow detects the prefix and publishes.

| Binding    | Release tag         |
| ---------- | ------------------- |
| python     | `python-v0.3.2`     |
| typescript | `typescript-v0.3.2` |
| rust       | `rust-v0.3.1`       |
| csharp     | `csharp-v0.3.0`     |
| java       | `java-v0.3.0`       |
| kotlin     | `kotlin-v0.2.2`     |
| elixir     | `elixir-v0.3.0`     |
| swift      | `swift-v0.3.0`      |

Every publish workflow also has a **Run workflow** button (`workflow_dispatch`)
with a `dry_run` toggle for testing without pushing to the registry.

> Bump the version in the binding's manifest **before** tagging — the tag does not
> set the version, the manifest does.

## One-time setup

### 1. Trusted Publishing (OIDC) — no secrets to store

Register this repo as a trusted publisher on each registry. Values are the same
everywhere: **owner** `hivellm`, **repo** `umicp`.

- **PyPI** — https://pypi.org/manage/project/umicp-sdk/settings/publishing/
  Workflow `publish-python.yml`, environment `pypi`.
  (For the very first upload, use the "pending publisher" form since the project
  may not exist yet.)
- **npm** — npmjs.com → package `@hivehub/umicp-sdk` → Settings → Trusted Publisher →
  GitHub Actions, workflow `publish-npm.yml`. The scope `@hivehub` org must exist
  and allow the account to publish.
- **crates.io** — crate `umicp-sdk` → Settings → Trusted Publishing →
  repo `hivellm/umicp`, workflow `publish-rust.yml`, environment `crates-io`.
  (First publish of a brand-new crate still needs a one-time manual `cargo publish`
  with a token to create the crate; trusted publishing takes over afterward.)
- **NuGet.org** — Account → Trusted Publishing → new policy for package
  `HiveLLM.Umicp.SDK`, repo `hivellm/umicp`, workflow `publish-nuget.yml`.
  Copy the generated **user** value into a repo **Variable** named `NUGET_USER`
  (Settings → Secrets and variables → Actions → Variables).

### 2. GitHub Environments

Create two environments (Settings → Environments) so the OIDC subject matches and
you can add required reviewers if desired:

- `pypi` (used by `publish-python.yml`)
- `crates-io` (used by `publish-rust.yml`)

### 3. Secrets (token-based registries only)

Add under Settings → Secrets and variables → Actions → **Secrets**:

| Secret                  | Used by            | How to get it                                                        |
| ----------------------- | ------------------ | -------------------------------------------------------------------- |
| `CENTRAL_USERNAME`      | Maven (java+kotlin)| Central Portal → Generate User Token (name)                          |
| `CENTRAL_PASSWORD`      | Maven (java+kotlin)| Central Portal → Generate User Token (password)                      |
| `MAVEN_GPG_PRIVATE_KEY` | Maven (java+kotlin)| `gpg --armor --export-secret-keys <KEYID>` (whole block)             |
| `MAVEN_GPG_PASSPHRASE`  | Maven (java+kotlin)| passphrase for that GPG key                                          |
| `HEX_API_KEY`           | Elixir             | `mix hex.user key generate --permission api:write`                   |

And under **Variables**:

| Variable     | Used by | Value                                                    |
| ------------ | ------- | -------------------------------------------------------- |
| `NUGET_USER` | NuGet   | the `user` value from the NuGet trusted-publishing policy |

Swift needs nothing — consumers depend on the git tag directly.

## Notes / caveats

- **CI is real**, not advisory. Lint/type/format steps that this repo has never
  enforced may fail on the first run; a few soft-gates (`mypy`, `clippy`,
  `dotnet format`, `credo`) are set to `continue-on-error` so a style nit doesn't
  block a merge — tighten these to hard failures once the code is clean.
- **Kotlin version** in `bindings/kotlin/build.gradle.kts` is `0.2.2`, out of step
  with the other bindings; align it before the first tagged release.
- **Kotlin publishing** assumes `maven-publish` is wired to the Central Portal with
  a `publish` task reading the `ORG_GRADLE_PROJECT_*` credentials. Verify the
  `build.gradle.kts` `publishing {}`/`signing {}` blocks before the first deploy.
- **Java** deploy assumes a `release` profile in `pom.xml` (central-publishing +
  gpg + javadoc/sources). Add it if missing.
- Path filters keep each CI workflow scoped to its binding, so a change to one
  binding never triggers the whole matrix.
