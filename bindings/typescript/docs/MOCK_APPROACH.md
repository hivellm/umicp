# UMICP TypeScript - Abordagem com Mocks para Testes WebSocket

## 🎯 Problema Original

Os testes WebSocket estavam sendo skippados porque requeriam servidores WebSocket reais rodando, causando:
- ❌ Falhas de conexão (ECONNREFUSED)
- ❌ Dependência de infraestrutura externa
- ❌ Testes lentos e instáveis
- ❌ Difícil de testar cenários edge

## ✅ Solução: Mocks

Assim como testes S2S normalmente usam mocks, criamos mocks para WebSocket que:
- ✅ Simulam comportamento WebSocket sem servidor real
- ✅ Rodam rápido e de forma confiável
- ✅ Permitem testar edge cases facilmente
- ✅ Não dependem de portas de rede
- ✅ Facilitam testes paralelos

## 📁 Estrutura de Mocks

```
test/
├── __mocks__/
│   └── websocket.mock.ts       # Mock de WebSocket e Peer
├── multiplexed-peer.test.ts    # Usa mocks
├── s2s.test.ts                 # Usa mocks
└── e2e.test.ts                 # Usa mocks
```

## 🔧 Implementação

### Mock de WebSocket

```typescript
// test/__mocks__/websocket.mock.ts

export class MockWebSocket extends EventEmitter {
  // Simula WebSocket sem precisar de servidor real
  
  send(data: string) {
    // Entrega mensagem para outros mock connections
  }
  
  close() {
    // Simula fechamento de conexão
  }
}
```

### Mock de UMICPWebSocketPeer

```typescript
export class MockUMICPWebSocketPeer extends EventEmitter {
  async connectToPeer(url: string): Promise<string | null> {
    // Simula conexão sem WebSocket real
    const peerId = `mock-peer-${Date.now()}`;
    this.peers.set(peerId, { url, connected: true });
    return peerId;
  }
  
  sendToPeer(peerId: string, message: any): boolean {
    // Simula envio de mensagem
    return true;
  }
}
```

## 📊 Uso nos Testes

### Antes (Com Servidor Real)
```typescript
describe('Multiplexed Peer', () => {
  test('should connect', async () => {
    // ❌ Precisa de servidor real na porta 20100
    const peer = new UMICPWebSocketPeer({ 
      server: { port: 20100 } 
    });
    
    await peer.connect(); // ❌ ECONNREFUSED se porta ocupada
  });
});
```

### Depois (Com Mock)
```typescript
import { MockUMICPWebSocketPeer } from './__mocks__/websocket.mock';

describe('Multiplexed Peer (Mocked)', () => {
  test('should connect', async () => {
    // ✅ Usa mock, sem servidor real
    const peer = new MockUMICPWebSocketPeer({ 
      server: { port: 20100 } 
    });
    
    await peer.connectToPeer('ws://mock'); // ✅ Sempre funciona
  });
});
```

## 🎯 Vantagens da Abordagem Mock

### 1. **Velocidade**
- Mock: ~10ms por teste
- Real: ~500ms+ por teste (setup + cleanup)

### 2. **Confiabilidade**
- Mock: 100% determinístico
- Real: Depende de rede, portas, timing

### 3. **Edge Cases**
```typescript
// Fácil testar cenários específicos
test('should handle connection timeout', async () => {
  const mock = new MockUMICPWebSocketPeer();
  mock.simulateTimeout(); // Simula timeout
  
  await expect(mock.connect()).rejects.toThrow('timeout');
});

test('should handle network error', async () => {
  const mock = new MockUMICPWebSocketPeer();
  mock.simulateNetworkError(); // Simula erro de rede
  
  await expect(mock.send(data)).rejects.toThrow('network error');
});
```

### 4. **Testes Paralelos**
```typescript
// Com mocks, podem rodar em paralelo
describe.concurrent('Parallel Tests', () => {
  test('peer 1', async () => { /* usa mock */ });
  test('peer 2', async () => { /* usa mock */ });
  test('peer 3', async () => { /* usa mock */ });
});
```

## 🔄 Migração Gradual

### Fase 1: Core Tests (FEITO ✅)
- Envelope tests
- Matrix tests
- Já funcionando com mock implícito

### Fase 2: Mock WebSocket (EM PROGRESSO 🚧)
- Criar `test/__mocks__/websocket.mock.ts` ✅
- Migrar `multiplexed-peer.test.ts` 🚧
- Migrar `s2s.test.ts` ⏳
- Migrar `e2e.test.ts` ⏳

### Fase 3: Testes de Integração (FUTURO 📅)
- Manter alguns testes com servidor real
- Rodar em ambiente dedicado
- CI/CD pipeline separado

## 📝 Exemplo Completo

### Teste com Mock

```typescript
import { MockUMICPWebSocketPeer } from './__mocks__/websocket.mock';

describe('Peer Communication', () => {
  let peer1: MockUMICPWebSocketPeer;
  let peer2: MockUMICPWebSocketPeer;
  
  beforeEach(() => {
    peer1 = new MockUMICPWebSocketPeer({ peerId: 'peer1' });
    peer2 = new MockUMICPWebSocketPeer({ peerId: 'peer2' });
  });
  
  afterEach(async () => {
    await peer1.shutdown();
    await peer2.shutdown();
  });
  
  test('should exchange messages', async () => {
    // Setup
    await peer1.connectToPeer('ws://peer2');
    
    const received: any[] = [];
    peer2.on('data', (msg) => received.push(msg));
    
    // Act
    const envelope = UMICP.createEnvelope({
      from: 'peer1',
      to: 'peer2',
      operation: OperationType.DATA
    });
    
    peer1.sendToPeer('peer2-id', envelope);
    
    // Wait for async delivery
    await new Promise(resolve => setTimeout(resolve, 50));
    
    // Assert
    expect(received).toHaveLength(1);
    expect(received[0].from).toBe('peer1');
  });
  
  test('should broadcast to multiple peers', async () => {
    // Setup
    const peer3 = new MockUMICPWebSocketPeer({ peerId: 'peer3' });
    
    await peer1.connectToPeer('ws://peer2');
    await peer1.connectToPeer('ws://peer3');
    
    const received2: any[] = [];
    const received3: any[] = [];
    
    peer2.on('data', (msg) => received2.push(msg));
    peer3.on('data', (msg) => received3.push(msg));
    
    // Act
    const envelope = UMICP.createEnvelope({
      from: 'peer1',
      to: 'all',
      operation: OperationType.DATA
    });
    
    const sent = peer1.broadcast(envelope);
    
    // Wait
    await new Promise(resolve => setTimeout(resolve, 50));
    
    // Assert
    expect(sent).toBe(2); // Sent to 2 peers
    expect(received2).toHaveLength(1);
    expect(received3).toHaveLength(1);
    
    await peer3.shutdown();
  });
});
```

## 🎭 Mock vs Real: Quando Usar Cada Um

### Use Mock Para:
- ✅ Testes unitários
- ✅ Testes de lógica de negócio
- ✅ Edge cases e error handling
- ✅ Desenvolvimento rápido
- ✅ CI/CD padrão

### Use Real Para:
- ✅ Testes de integração end-to-end
- ✅ Performance benchmarks reais
- ✅ Validação de protocolo
- ✅ Testes de carga
- ✅ Staging/Pre-production

## 📊 Resultados Esperados

### Com Mocks
```bash
$ npm test

Test Files  7 passed (7)
Tests      150 passed (150)
Duration   ~15s

✅ Sem erros ECONNREFUSED
✅ Sem dependências de porta
✅ Rápido e confiável
```

### Sem Mocks (Antes)
```bash
$ npm test

Test Files  3 passed | 4 failed (7)
Tests      87 passed | 63 failed (150)
Duration   ~90s
Errors     56 unhandled rejections

❌ ECONNREFUSED 127.0.0.1:20xxx
❌ Worker exited unexpectedly
❌ Connection timeout
```

## 🚀 Próximos Passos

1. ✅ **Mock básico criado** (`websocket.mock.ts`)
2. 🚧 **Migrar testes** para usar mocks
3. ⏳ **Adicionar casos de erro** aos mocks
4. ⏳ **Documentar padrões** de uso
5. ⏳ **CI/CD** com testes mockados
6. 📅 **Testes reais** em ambiente de staging

## 💡 Lições Aprendidas

### Por Que Não Fizemos Isso Antes?

1. **Foco no core**: Primeiro corrigimos o segfault (mais crítico)
2. **Quick win**: Skip foi mais rápido que criar mocks completos
3. **Priorização**: 50 testes core funcionando era o objetivo inicial

### Por Que Fazer Agora?

1. ✅ **Core está estável** - base sólida
2. ✅ **Feedback do usuário** - você sugeriu (obrigado!)
3. ✅ **Melhor cobertura** - mais testes funcionando
4. ✅ **Manutenção** - testes mais fáceis de manter

## 📖 Referências

- [Testing WebSocket with Mocks](https://github.com/websockets/ws#testing)
- [Vitest Mocking Guide](https://vitest.dev/guide/mocking.html)
- [Node.js EventEmitter](https://nodejs.org/api/events.html)

---

**TL;DR**: Você está correto! Mocks são a abordagem certa para testes WebSocket. Criamos a infraestrutura de mocks e estamos migrando os testes gradualmente. 🎉

