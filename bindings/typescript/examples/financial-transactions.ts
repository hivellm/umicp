/**
 * UMICP TypeScript Bindings - Financial Transactions Example
 * Demonstrates secure financial transaction processing using UMICP envelopes
 */

import { Envelope, UMICP, OperationType, PayloadType, EncodingType } from '../src/index.js';
import * as crypto from 'crypto';

interface FinancialTransaction {
  transactionId: string;
  type: 'payment' | 'transfer' | 'withdrawal' | 'deposit' | 'refund';
  amount: number;
  currency: string;
  fromAccount: string;
  toAccount: string;
  timestamp: number;
  description: string;
  metadata: {
    merchantId?: string;
    terminalId?: string;
    location?: string;
    deviceFingerprint?: string;
    riskScore?: number;
  };
}

interface TransactionResult {
  success: boolean;
  transactionId: string;
  processingTime: number;
  fee: number;
  riskAssessment: RiskLevel;
  authorizationCode?: string;
  errorMessage?: string;
}

type RiskLevel = 'low' | 'medium' | 'high' | 'critical';

class FinancialTransactionProcessor {
  private transactionLog: FinancialTransaction[] = [];
  private processedTransactions: Map<string, TransactionResult> = new Map();

  // Process a financial transaction
  async processTransaction(transaction: FinancialTransaction): Promise<TransactionResult> {
    const startTime = Date.now();

    console.log(`💰 Processing ${transaction.type} transaction: ${transaction.transactionId}`);
    console.log(`   Amount: ${transaction.amount} ${transaction.currency}`);
    console.log(`   From: ${transaction.fromAccount} -> To: ${transaction.toAccount}`);

    // Validate transaction
    const validation = this.validateTransaction(transaction);
    if (!validation.isValid) {
      return {
        success: false,
        transactionId: transaction.transactionId,
        processingTime: Date.now() - startTime,
        fee: 0,
        riskAssessment: 'critical',
        errorMessage: validation.error
      };
    }

    // Risk assessment
    const riskLevel = this.assessRisk(transaction);

    // Calculate processing fee
    const fee = this.calculateFee(transaction, riskLevel);

    // Simulate processing delay based on risk
    const processingDelay = this.getProcessingDelay(riskLevel);
    await new Promise(resolve => setTimeout(resolve, processingDelay));

    // Generate authorization code
    const authCode = this.generateAuthorizationCode();

    // Create result
    const result: TransactionResult = {
      success: true,
      transactionId: transaction.transactionId,
      processingTime: Date.now() - startTime,
      fee,
      riskAssessment: riskLevel,
      authorizationCode: authCode
    };

    // Store transaction
    this.transactionLog.push(transaction);
    this.processedTransactions.set(transaction.transactionId, result);

    console.log(`✅ Transaction processed successfully`);
    console.log(`   Auth Code: ${authCode}`);
    console.log(`   Processing time: ${result.processingTime}ms`);
    console.log(`   Fee: ${fee} ${transaction.currency}`);
    console.log(`   Risk level: ${riskLevel.toUpperCase()}`);
    console.log();

    return result;
  }

  private validateTransaction(transaction: FinancialTransaction): { isValid: boolean; error?: string } {
    // Amount validation
    if (transaction.amount <= 0) {
      return { isValid: false, error: 'Invalid transaction amount' };
    }

    // Currency validation
    if (!['USD', 'EUR', 'GBP', 'JPY', 'BRL'].includes(transaction.currency)) {
      return { isValid: false, error: 'Unsupported currency' };
    }

    // Account validation
    if (!transaction.fromAccount || !transaction.toAccount) {
      return { isValid: false, error: 'Invalid account information' };
    }

    // Prevent self-transfers for certain types
    if (transaction.type === 'transfer' && transaction.fromAccount === transaction.toAccount) {
      return { isValid: false, error: 'Cannot transfer to the same account' };
    }

    return { isValid: true };
  }

  private assessRisk(transaction: FinancialTransaction): RiskLevel {
    let riskScore = 0;

    // Amount-based risk
    if (transaction.amount > 10000) riskScore += 30;
    else if (transaction.amount > 1000) riskScore += 15;
    else if (transaction.amount > 100) riskScore += 5;

    // Type-based risk
    if (transaction.type === 'withdrawal' && transaction.amount > 5000) {
      riskScore += 20;
    }

    // Location-based risk
    if (transaction.metadata.location === 'high-risk-country') {
      riskScore += 25;
    }

    // Time-based risk (unusual hours)
    const hour = new Date(transaction.timestamp).getHours();
    if (hour < 6 || hour > 22) {
      riskScore += 10;
    }

    // Device fingerprint risk
    if (!transaction.metadata.deviceFingerprint) {
      riskScore += 15;
    }

    if (riskScore >= 50) return 'critical';
    if (riskScore >= 30) return 'high';
    if (riskScore >= 15) return 'medium';
    return 'low';
  }

  private calculateFee(transaction: FinancialTransaction, riskLevel: RiskLevel): number {
    let baseFee = 0.0;

    // Base fee by transaction type
    switch (transaction.type) {
      case 'payment':
        baseFee = 0.029; // 2.9%
        break;
      case 'transfer':
        baseFee = 0.001; // 0.1%
        break;
      case 'withdrawal':
        baseFee = 2.50; // Fixed fee
        break;
      case 'deposit':
        baseFee = 0.0; // No fee
        break;
      case 'refund':
        baseFee = 1.00; // Fixed fee
        break;
    }

    // Risk-based fee adjustment
    let riskMultiplier = 1.0;
    switch (riskLevel) {
      case 'medium':
        riskMultiplier = 1.2;
        break;
      case 'high':
        riskMultiplier = 1.5;
        break;
      case 'critical':
        riskMultiplier = 2.0;
        break;
    }

    return transaction.type === 'withdrawal' || transaction.type === 'refund'
      ? baseFee * riskMultiplier
      : transaction.amount * baseFee * riskMultiplier;
  }

  private getProcessingDelay(riskLevel: RiskLevel): number {
    switch (riskLevel) {
      case 'low':
        return 50 + Math.random() * 100; // 50-150ms
      case 'medium':
        return 200 + Math.random() * 300; // 200-500ms
      case 'high':
        return 500 + Math.random() * 1000; // 500-1500ms
      case 'critical':
        return 2000 + Math.random() * 3000; // 2000-5000ms
      default:
        return 100;
    }
  }

  private generateAuthorizationCode(): string {
    return Math.random().toString(36).substring(2, 10).toUpperCase();
  }

  // Create secure transaction envelope
  createTransactionEnvelope(transaction: FinancialTransaction): any {
    // Create digital signature for transaction integrity
    const transactionData = JSON.stringify({
      id: transaction.transactionId,
      amount: transaction.amount,
      from: transaction.fromAccount,
      to: transaction.toAccount,
      timestamp: transaction.timestamp
    });

    const signature = crypto.createHmac('sha256', 'transaction-secret-key')
      .update(transactionData)
      .digest('hex');

    return UMICP.createEnvelope({
      from: `financial-gateway-${Date.now().toString().slice(-6)}`,
      to: 'transaction-processor',
      operation: OperationType.REQUEST,
      messageId: `txn-${transaction.transactionId}`,
      capabilities: {
        'message_type': 'financial_transaction',
        'transaction_type': transaction.type,
        'transaction_id': transaction.transactionId,
        'amount': transaction.amount.toString(),
        'currency': transaction.currency,
        'from_account': transaction.fromAccount,
        'to_account': transaction.toAccount,
        'timestamp': transaction.timestamp.toString(),
        'description': transaction.description,
        'merchant_id': transaction.metadata.merchantId || '',
        'terminal_id': transaction.metadata.terminalId || '',
        'location': transaction.metadata.location || '',
        'device_fingerprint': transaction.metadata.deviceFingerprint || '',
        'signature': signature,
        'protocol_version': '1.0'
      },
      payloadHint: {
        type: PayloadType.VECTOR,
        size: transactionData.length,
        encoding: EncodingType.UTF8,
        count: 1
      }
    });
  }

  // Get transaction statistics
  getTransactionStats(): any {
    const totalTransactions = this.transactionLog.length;
    const successfulTransactions = Array.from(this.processedTransactions.values())
      .filter(result => result.success).length;

    const totalAmount = this.transactionLog.reduce((sum, txn) => sum + txn.amount, 0);
    const totalFees = Array.from(this.processedTransactions.values())
      .filter(result => result.success)
      .reduce((sum, result) => sum + result.fee, 0);

    const avgProcessingTime = Array.from(this.processedTransactions.values())
      .reduce((sum, result) => sum + result.processingTime, 0) / this.processedTransactions.size;

    const riskDistribution = Array.from(this.processedTransactions.values())
      .reduce((dist, result) => {
        dist[result.riskAssessment] = (dist[result.riskAssessment] || 0) + 1;
        return dist;
      }, {} as Record<string, number>);

    return {
      totalTransactions,
      successfulTransactions,
      successRate: totalTransactions > 0 ? (successfulTransactions / totalTransactions) * 100 : 0,
      totalAmount,
      totalFees,
      avgProcessingTime: avgProcessingTime || 0,
      riskDistribution
    };
  }
}

class FinancialTransactionSimulator {
  private processor: FinancialTransactionProcessor;
  private accounts: Map<string, number> = new Map();

  constructor() {
    this.processor = new FinancialTransactionProcessor();
    this.initializeAccounts();
  }

  private initializeAccounts(): void {
    // Initialize some test accounts with balances
    this.accounts.set('acc-001', 10000); // John Doe
    this.accounts.set('acc-002', 5000);  // Jane Smith
    this.accounts.set('acc-003', 25000); // Business Corp
    this.accounts.set('acc-004', 1000);  // Small Account
    this.accounts.set('merchant-001', 0); // Merchant account
  }

  // Generate realistic financial transactions
  private generateTransaction(): FinancialTransaction {
    const types: FinancialTransaction['type'][] = ['payment', 'transfer', 'withdrawal', 'deposit'];
    const currencies = ['USD', 'EUR', 'GBP'];
    const accounts = Array.from(this.accounts.keys());

    const fromAccount = accounts[Math.floor(Math.random() * accounts.length)];
    let toAccount: string;
    do {
      toAccount = accounts[Math.floor(Math.random() * accounts.length)];
    } while (toAccount === fromAccount);

    const transaction: FinancialTransaction = {
      transactionId: `txn-${Date.now()}-${Math.random().toString(36).substr(2, 6)}`,
      type: types[Math.floor(Math.random() * types.length)],
      amount: Math.random() * 1000 + 10, // $10 to $1010
      currency: currencies[Math.floor(Math.random() * currencies.length)],
      fromAccount,
      toAccount,
      timestamp: Date.now(),
      description: `Transaction ${Math.random().toString(36).substr(2, 8)}`,
      metadata: {
        merchantId: Math.random() > 0.7 ? 'merchant-001' : undefined,
        terminalId: Math.random() > 0.5 ? `term-${Math.random().toString(36).substr(2, 4)}` : undefined,
        location: Math.random() > 0.6 ? 'US' : 'EU',
        deviceFingerprint: Math.random() > 0.3 ? crypto.randomBytes(16).toString('hex') : undefined,
        riskScore: Math.random() * 100
      }
    };

    return transaction;
  }

  // Run transaction simulation
  async runSimulation(transactionCount: number = 20): Promise<void> {
    console.log(`🏦 Starting financial transaction simulation`);
    console.log(`   Processing ${transactionCount} transactions...`);
    console.log();

    const envelopes = [];

    for (let i = 0; i < transactionCount; i++) {
      const transaction = this.generateTransaction();

      // Create secure envelope
      const envelope = this.processor.createTransactionEnvelope(transaction);
      envelopes.push({ transaction, envelope });

      // Process transaction
      await this.processor.processTransaction(transaction);

      // Small delay between transactions
      await new Promise(resolve => setTimeout(resolve, 100));
    }

    console.log(`📊 Processing ${envelopes.length} secure envelopes...`);
    console.log();
  }

  getProcessor(): FinancialTransactionProcessor {
    return this.processor;
  }
}

async function financialTransactionsExample() {
  console.log('💳 UMICP Financial Transactions Example');
  console.log('=======================================');;

  const simulator = new FinancialTransactionSimulator();
  await simulator.runSimulation(15);

  // Show final statistics
  const stats = simulator.getProcessor().getTransactionStats();

  console.log('🎉 Financial Transaction Processing Complete!');
  console.log('=============================================');
  console.log(`📊 Final Statistics:`);
  console.log(`   Total transactions: ${stats.totalTransactions}`);
  console.log(`   Successful transactions: ${stats.successfulTransactions}`);
  console.log(`   Success rate: ${stats.successRate.toFixed(2)}%`);
  console.log(`   Total amount processed: $${stats.totalAmount.toFixed(2)}`);
  console.log(`   Total fees collected: $${stats.totalFees.toFixed(2)}`);
  console.log(`   Average processing time: ${stats.avgProcessingTime.toFixed(2)}ms`);
  console.log(`   Risk distribution:`, JSON.stringify(stats.riskDistribution, null, 2));
  console.log();
  console.log('💡 Key Benefits of UMICP in Financial Services:');
  console.log('   • Secure transaction envelope creation');
  console.log('   • Digital signature verification');
  console.log('   • Real-time risk assessment');
  console.log('   • Type-safe financial data handling');
  console.log('   • High-performance transaction processing');
}

// Run example if executed directly
if (require.main === module) {
  financialTransactionsExample().catch(console.error);
}

export { FinancialTransactionProcessor, FinancialTransactionSimulator, financialTransactionsExample };

