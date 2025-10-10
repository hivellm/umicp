// UMICP Java Bindings - Gradle Build Script (Alternative to Maven)

plugins {
    java
    `java-library`
    `maven-publish`
    signing
    jacoco
    id("com.github.johnrengelman.shadow") version "8.1.1"
}

group = "com.hivellm"
version = "1.0.0-SNAPSHOT"

java {
    sourceCompatibility = JavaVersion.VERSION_17
    targetCompatibility = JavaVersion.VERSION_17
    withSourcesJar()
    withJavadocJar()
}

repositories {
    mavenCentral()
}

dependencies {
    // Logging
    implementation("org.slf4j:slf4j-api:2.0.9")
    implementation("ch.qos.logback:logback-classic:1.4.14")

    // JSON Processing
    implementation("com.fasterxml.jackson.core:jackson-databind:2.16.0")

    // WebSocket
    implementation("org.java-websocket:Java-WebSocket:1.5.5")

    // HTTP Client
    implementation("com.squareup.okhttp3:okhttp:4.12.0")

    // Netty (Alternative for WebSocket/HTTP2)
    implementation("io.netty:netty-all:4.1.104.Final")

    // Annotations
    compileOnly("org.jetbrains:annotations:24.1.0")

    // Testing
    testImplementation(platform("org.junit:junit-bom:5.10.1"))
    testImplementation("org.junit.jupiter:junit-jupiter")
    testImplementation("org.mockito:mockito-core:5.8.0")
    testImplementation("org.mockito:mockito-junit-jupiter:5.8.0")
    testImplementation("org.assertj:assertj-core:3.25.1")

    // Benchmarking
    testImplementation("org.openjdk.jmh:jmh-core:1.37")
    testAnnotationProcessor("org.openjdk.jmh:jmh-generator-annprocess:1.37")
}

tasks.test {
    useJUnitPlatform()
    maxHeapSize = "2g"

    testLogging {
        events("passed", "skipped", "failed")
        showStandardStreams = false
    }
}

tasks.jacocoTestReport {
    dependsOn(tasks.test)

    reports {
        xml.required.set(true)
        html.required.set(true)
    }
}

tasks.jacocoTestCoverageVerification {
    violationRules {
        rule {
            limit {
                minimum = "0.80".toBigDecimal()
            }
        }
    }
}

tasks.javadoc {
    options {
        encoding = "UTF-8"
        (this as StandardJavadocDocletOptions).apply {
            addStringOption("Xdoclint:none", "-quiet")
            links("https://docs.oracle.com/en/java/javase/17/docs/api/")
        }
    }
}

tasks.jar {
    manifest {
        attributes(
            "Implementation-Title" to project.name,
            "Implementation-Version" to project.version,
            "Built-By" to System.getProperty("user.name"),
            "Built-JDK" to System.getProperty("java.version")
        )
    }
}

publishing {
    publications {
        create<MavenPublication>("mavenJava") {
            from(components["java"])

            pom {
                name.set("UMICP Java Bindings")
                description.set("Java bindings for Universal Matrix Inter-Communication Protocol")
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
            val releasesRepoUrl = uri("https://s01.oss.sonatype.org/service/local/staging/deploy/maven2/")
            val snapshotsRepoUrl = uri("https://s01.oss.sonatype.org/content/repositories/snapshots")
            url = if (version.toString().endsWith("SNAPSHOT")) snapshotsRepoUrl else releasesRepoUrl

            credentials {
                username = findProperty("ossrhUsername")?.toString() ?: System.getenv("OSSRH_USERNAME")
                password = findProperty("ossrhPassword")?.toString() ?: System.getenv("OSSRH_PASSWORD")
            }
        }
    }
}

signing {
    sign(publishing.publications["mavenJava"])
}

// Custom tasks
tasks.register("benchmark") {
    group = "verification"
    description = "Run JMH benchmarks"
    dependsOn(tasks.test)
}

tasks.register("coverage") {
    group = "verification"
    description = "Generate coverage report"
    dependsOn(tasks.jacocoTestReport, tasks.jacocoTestCoverageVerification)
}

tasks.register("quality") {
    group = "verification"
    description = "Run all quality checks"
    dependsOn(tasks.test, tasks.jacocoTestCoverageVerification, tasks.javadoc)
}

