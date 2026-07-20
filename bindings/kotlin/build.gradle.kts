import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

plugins {
    kotlin("jvm") version "1.9.21"
    kotlin("plugin.serialization") version "1.9.21"
    id("org.jetbrains.dokka") version "1.9.10"
    id("maven-publish")
    id("signing")
    jacoco
}

group = "com.hivellm.org"
version = "0.2.2"

repositories {
    mavenCentral()
}

dependencies {
    // Kotlin Standard Library
    implementation(kotlin("stdlib"))
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:1.7.3")

    // Serialization
    implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.6.2")
    implementation("com.google.code.gson:gson:2.10.1")

    // WebSocket
    implementation("org.java-websocket:Java-WebSocket:1.5.5")

    // HTTP Client
    implementation("com.squareup.okhttp3:okhttp:4.12.0")
    implementation("com.squareup.okhttp3:okhttp-sse:4.12.0")

    // Compression
    implementation("org.apache.commons:commons-compress:1.25.0")
    implementation("org.lz4:lz4-java:1.8.0")

    // HTTP/2 Server
    implementation("io.undertow:undertow-core:2.3.10.Final")

    // Logging
    implementation("io.github.microutils:kotlin-logging-jvm:3.0.5")
    implementation("ch.qos.logback:logback-classic:1.4.14")

    // Testing
    testImplementation(kotlin("test"))
    testImplementation("org.jetbrains.kotlinx:kotlinx-coroutines-test:1.7.3")
    testImplementation("io.kotest:kotest-runner-junit5:5.8.0")
    testImplementation("io.kotest:kotest-assertions-core:5.8.0")
    testImplementation("io.mockk:mockk:1.13.9")
    testImplementation("org.junit.jupiter:junit-jupiter:5.10.1")
}

tasks.test {
    useJUnitPlatform()
    // TODO: HttpClientTest/HttpServerTest are integration tests that require a live
    // HTTP server/free port and don't run reliably in CI. Excluded until they are
    // rewritten against a mock/in-process server.
    filter {
        excludeTestsMatching("*HttpClientTest")
        excludeTestsMatching("*HttpServerTest")
        isFailOnNoMatchingTests = false
    }
    finalizedBy(tasks.jacocoTestReport)
}

tasks.jacocoTestReport {
    dependsOn(tasks.test)
    reports {
        xml.required.set(true)
        html.required.set(true)
    }
}

tasks.withType<KotlinCompile> {
    kotlinOptions {
        freeCompilerArgs = listOf("-Xjsr305=strict")
        jvmTarget = "17"
    }
}

java {
    sourceCompatibility = JavaVersion.VERSION_17
    targetCompatibility = JavaVersion.VERSION_17
    withSourcesJar()
    withJavadocJar()
}

publishing {
    publications {
        create<MavenPublication>("maven") {
            from(components["java"])

            pom {
                name.set("UMICP Kotlin SDK")
                description.set("Kotlin SDK for Universal Matrix Intelligent Communication Protocol")
                url.set("https://github.com/hivellm/umicp")

                licenses {
                    license {
                        name.set("MIT License")
                        url.set("https://opensource.org/licenses/MIT")
                    }
                }

                developers {
                    developer {
                        name.set("HiveLLM Team")
                        organization.set("HiveLLM")
                        organizationUrl.set("https://github.com/hivellm")
                    }
                }

                scm {
                    connection.set("scm:git:git://github.com/hivellm/umicp.git")
                    developerConnection.set("scm:git:ssh://github.com:hivellm/umicp.git")
                    url.set("https://github.com/hivellm/umicp/tree/main")
                }
            }
        }
    }
    repositories {
        maven {
            name = "CentralPortal"
            url = uri("https://central.sonatype.com/api/v1/publisher/deployments/download")

            credentials {
                username = findProperty("centralUsername") as String? ?: System.getenv("ORG_GRADLE_PROJECT_centralUsername") ?: "andreferreira"
                password = findProperty("centralToken") as String? ?: System.getenv("ORG_GRADLE_PROJECT_centralToken") ?: "csbk8dyyw4"
            }
        }
    }
}

signing {
    val signingKeyId: String? = findProperty("signingKeyId") as String? ?: System.getenv("ORG_GRADLE_PROJECT_signingKeyId")
    val signingKey: String? = findProperty("signingKey") as String? ?: System.getenv("ORG_GRADLE_PROJECT_signingKey")
    val signingPassword: String? = findProperty("signingPassword") as String? ?: System.getenv("ORG_GRADLE_PROJECT_signingPassword")

    if (signingKeyId != null && signingKey != null && signingPassword != null) {
        useInMemoryPgpKeys(signingKeyId, signingKey, signingPassword)
        sign(publishing.publications["maven"])
    }
}

tasks.dokkaHtml {
    outputDirectory.set(layout.buildDirectory.dir("docs"))
}

