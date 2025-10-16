package com.hivellm.umicp.discovery;

import java.util.ArrayList;
import java.util.List;

/**
 * Simple implementation of DiscoverableService.
 * Provides a straightforward way to create a discoverable service with a fixed set of operations.
 *
 * @author HiveLLM Team
 * @version 0.2.0
 * @since 0.2.0
 */
public class SimpleDiscoverableService implements DiscoverableService {
    
    private final List<OperationSchema> operations;
    private final ServerInfo serverInfo;
    
    /**
     * Constructs a SimpleDiscoverableService.
     *
     * @param operations List of operation schemas
     * @param serverInfo Server information
     */
    public SimpleDiscoverableService(List<OperationSchema> operations, ServerInfo serverInfo) {
        if (operations == null) {
            throw new IllegalArgumentException("Operations list cannot be null");
        }
        if (serverInfo == null) {
            throw new IllegalArgumentException("Server info cannot be null");
        }
        
        this.operations = new ArrayList<>(operations);
        this.serverInfo = serverInfo;
    }
    
    @Override
    public List<OperationSchema> listOperations() {
        return new ArrayList<>(operations);
    }
    
    @Override
    public OperationSchema getSchema(String name) {
        if (name == null) {
            return null;
        }
        
        for (OperationSchema op : operations) {
            if (name.equals(op.getName())) {
                return op;
            }
        }
        
        return null;
    }
    
    @Override
    public ServerInfo getServerInfo() {
        // Return a copy with updated operations count
        return new ServerInfo(
                serverInfo.getServer(),
                serverInfo.getVersion(),
                serverInfo.getProtocol(),
                serverInfo.getFeatures(),
                operations.size(),
                serverInfo.getMcpCompatible(),
                serverInfo.getMetadata()
        );
    }
}

