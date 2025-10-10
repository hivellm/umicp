package com.hivellm.umicp.transport.http;

import java.time.Duration;

/**
 * Configuration options for HTTP/2 client
 */
public class HttpClientOptions {
    private String baseUrl;
    private Duration connectTimeout;
    private Duration requestTimeout;
    private boolean followRedirects;
    private boolean useHttp2;
    private int maxRetries;

    public HttpClientOptions() {
        this.connectTimeout = Duration.ofSeconds(10);
        this.requestTimeout = Duration.ofSeconds(30);
        this.followRedirects = true;
        this.useHttp2 = true;  // Enable HTTP/2 by default
        this.maxRetries = 3;
    }

    // Getters
    public String getBaseUrl() { return baseUrl; }
    public Duration getConnectTimeout() { return connectTimeout; }
    public Duration getRequestTimeout() { return requestTimeout; }
    public boolean isFollowRedirects() { return followRedirects; }
    public boolean isUseHttp2() { return useHttp2; }
    public int getMaxRetries() { return maxRetries; }

    // Setters with fluent API
    public HttpClientOptions setBaseUrl(String baseUrl) {
        this.baseUrl = baseUrl;
        return this;
    }

    public HttpClientOptions setConnectTimeout(Duration connectTimeout) {
        this.connectTimeout = connectTimeout;
        return this;
    }

    public HttpClientOptions setRequestTimeout(Duration requestTimeout) {
        this.requestTimeout = requestTimeout;
        return this;
    }

    public HttpClientOptions setFollowRedirects(boolean followRedirects) {
        this.followRedirects = followRedirects;
        return this;
    }

    public HttpClientOptions setUseHttp2(boolean useHttp2) {
        this.useHttp2 = useHttp2;
        return this;
    }

    public HttpClientOptions setMaxRetries(int maxRetries) {
        if (maxRetries < 0) {
            throw new IllegalArgumentException("maxRetries must be >= 0");
        }
        this.maxRetries = maxRetries;
        return this;
    }
}

