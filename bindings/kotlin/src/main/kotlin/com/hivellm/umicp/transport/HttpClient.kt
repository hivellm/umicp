package com.hivellm.umicp.transport

import com.hivellm.umicp.core.Envelope
import com.hivellm.umicp.types.TransportException
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import mu.KotlinLogging
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import java.time.Duration
import java.util.concurrent.TimeUnit

private val logger = KotlinLogging.logger {}

/**
 * HTTP/2 Client for UMICP
 * 
 * Provides HTTP/2 transport with automatic protocol negotiation.
 * 
 * @property baseUrl Base URL of the server
 * @property timeout Request timeout in milliseconds
 */
class UMICPHttpClient(
    private val baseUrl: String,
    private val timeout: Long = 30_000
) : AutoCloseable {
    
    private val client: OkHttpClient = OkHttpClient.Builder()
        .connectTimeout(timeout, TimeUnit.MILLISECONDS)
        .readTimeout(timeout, TimeUnit.MILLISECONDS)
        .writeTimeout(timeout, TimeUnit.MILLISECONDS)
        .protocols(listOf(
            okhttp3.Protocol.HTTP_2,
            okhttp3.Protocol.HTTP_1_1
        ))
        .build()
    
    private val jsonMediaType = "application/json; charset=utf-8".toMediaType()
    
    /**
     * Send an envelope via HTTP POST
     * 
     * @param endpoint API endpoint (e.g., "/api/messages")
     * @param envelope Envelope to send
     * @return Response envelope
     * @throws TransportException if request fails
     */
    suspend fun post(endpoint: String, envelope: Envelope): Envelope = withContext(Dispatchers.IO) {
        try {
            val url = "$baseUrl$endpoint"
            val json = envelope.serialize()
            
            val request = Request.Builder()
                .url(url)
                .post(json.toRequestBody(jsonMediaType))
                .header("Content-Type", "application/json")
                .header("Accept", "application/json")
                .build()
            
            logger.debug { "POST $url" }
            
            val response = client.newCall(request).execute()
            
            if (!response.isSuccessful) {
                throw TransportException("HTTP ${response.code}: ${response.message}")
            }
            
            val responseBody = response.body?.string()
                ?: throw TransportException("Empty response body")
            
            logger.debug { "Response: ${response.code} (Protocol: ${response.protocol})" }
            
            Envelope.deserialize(responseBody)
        } catch (e: Exception) {
            if (e is TransportException) throw e
            throw TransportException("Failed to POST envelope", e)
        }
    }
    
    /**
     * Send an envelope via HTTP GET
     * 
     * @param endpoint API endpoint with query parameters
     * @return Response envelope
     * @throws TransportException if request fails
     */
    suspend fun get(endpoint: String): Envelope = withContext(Dispatchers.IO) {
        try {
            val url = "$baseUrl$endpoint"
            
            val request = Request.Builder()
                .url(url)
                .get()
                .header("Accept", "application/json")
                .build()
            
            logger.debug { "GET $url" }
            
            val response = client.newCall(request).execute()
            
            if (!response.isSuccessful) {
                throw TransportException("HTTP ${response.code}: ${response.message}")
            }
            
            val responseBody = response.body?.string()
                ?: throw TransportException("Empty response body")
            
            logger.debug { "Response: ${response.code} (Protocol: ${response.protocol})" }
            
            Envelope.deserialize(responseBody)
        } catch (e: Exception) {
            if (e is TransportException) throw e
            throw TransportException("Failed to GET envelope", e)
        }
    }
    
    /**
     * Send an envelope via HTTP PUT
     * 
     * @param endpoint API endpoint
     * @param envelope Envelope to send
     * @return Response envelope
     * @throws TransportException if request fails
     */
    suspend fun put(endpoint: String, envelope: Envelope): Envelope = withContext(Dispatchers.IO) {
        try {
            val url = "$baseUrl$endpoint"
            val json = envelope.serialize()
            
            val request = Request.Builder()
                .url(url)
                .put(json.toRequestBody(jsonMediaType))
                .header("Content-Type", "application/json")
                .header("Accept", "application/json")
                .build()
            
            logger.debug { "PUT $url" }
            
            val response = client.newCall(request).execute()
            
            if (!response.isSuccessful) {
                throw TransportException("HTTP ${response.code}: ${response.message}")
            }
            
            val responseBody = response.body?.string()
                ?: throw TransportException("Empty response body")
            
            Envelope.deserialize(responseBody)
        } catch (e: Exception) {
            if (e is TransportException) throw e
            throw TransportException("Failed to PUT envelope", e)
        }
    }
    
    /**
     * Delete resource via HTTP DELETE
     * 
     * @param endpoint API endpoint
     * @return Response envelope
     * @throws TransportException if request fails
     */
    suspend fun delete(endpoint: String): Envelope = withContext(Dispatchers.IO) {
        try {
            val url = "$baseUrl$endpoint"
            
            val request = Request.Builder()
                .url(url)
                .delete()
                .header("Accept", "application/json")
                .build()
            
            logger.debug { "DELETE $url" }
            
            val response = client.newCall(request).execute()
            
            if (!response.isSuccessful) {
                throw TransportException("HTTP ${response.code}: ${response.message}")
            }
            
            val responseBody = response.body?.string()
                ?: throw TransportException("Empty response body")
            
            Envelope.deserialize(responseBody)
        } catch (e: Exception) {
            if (e is TransportException) throw e
            throw TransportException("Failed to DELETE resource", e)
        }
    }
    
    /**
     * Check if HTTP/2 is being used
     * 
     * @param endpoint Endpoint to check
     * @return true if HTTP/2 is available
     */
    suspend fun isHttp2(): Boolean = withContext(Dispatchers.IO) {
        try {
            val request = Request.Builder()
                .url(baseUrl)
                .head()
                .build()
            
            val response = client.newCall(request).execute()
            response.protocol == okhttp3.Protocol.HTTP_2
        } catch (e: Exception) {
            false
        }
    }
    
    override fun close() {
        client.dispatcher.executorService.shutdown()
        client.connectionPool.evictAll()
    }
    
    companion object {
        /**
         * Create HTTP client with DSL
         */
        inline fun create(baseUrl: String, timeout: Long = 30_000, block: UMICPHttpClient.() -> Unit = {}): UMICPHttpClient {
            return UMICPHttpClient(baseUrl, timeout).apply(block)
        }
    }
}

