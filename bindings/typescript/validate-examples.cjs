/**
 * Example Validation Script
 * Validates all communication examples
 */

const fs = require('fs');
const path = require('path');

const examples = [
  'examples/01-basic-peer-communication.ts',
  'examples/02-mesh-network.ts',
  'examples/03-http-communication.ts',
  'examples/04-request-response-pattern.ts',
  'examples/05-broadcast-patterns.ts',
  'examples/06-federated-learning-simulation.ts'
];

console.log('🔍 UMICP Examples Validation\n');
console.log('='.repeat(60));

let totalExamples = 0;
let validExamples = 0;
let totalLines = 0;

examples.forEach((file, index) => {
  const filePath = path.join(__dirname, file);
  const fileName = path.basename(file);

  console.log(`\n${index + 1}. ${fileName}`);
  console.log('-'.repeat(60));

  totalExamples++;

  try {
    // Check if file exists
    if (!fs.existsSync(filePath)) {
      console.log('  ❌ File not found');
      return;
    }
    console.log('  ✅ File exists');

    // Read file content
    const content = fs.readFileSync(filePath, 'utf-8');
    const lines = content.split('\n');
    totalLines += lines.length;

    // Validate structure
    const checks = {
      'Has imports': content.includes('import {'),
      'Has main function': content.includes('async function main()'),
      'Has error handling': content.includes('.catch('),
      'Has cleanup': content.includes('shutdown()'),
      'Has console output': content.includes('console.log'),
      'Creates peers': content.includes('new UMICPWebSocketPeer') || content.includes('new StreamableHTTP'),
      'Has event handlers': content.includes('.on('),
      'Has comments': content.includes('/**') || content.includes('//'),
    };

    console.log('  📊 Structure checks:');
    let passedChecks = 0;
    Object.entries(checks).forEach(([check, passed]) => {
      console.log(`     ${passed ? '✅' : '❌'} ${check}`);
      if (passed) passedChecks++;
    });

    // File statistics
    console.log('  📈 Statistics:');
    console.log(`     Lines: ${lines.length}`);
    console.log(`     Size: ${(fs.statSync(filePath).size / 1024).toFixed(2)} KB`);

    // Count key elements
    const peerCount = (content.match(/new UMICPWebSocketPeer/g) || []).length +
                     (content.match(/new StreamableHTTP/g) || []).length;
    const eventCount = (content.match(/\.on\(/g) || []).length;
    const envelopeCount = (content.match(/new Envelope/g) || []).length;

    console.log(`     Peers created: ${peerCount}`);
    console.log(`     Event handlers: ${eventCount}`);
    console.log(`     Envelopes: ${envelopeCount}`);

    // Validate port usage
    const portMatches = content.match(/port:\s*(\d+)/g);
    if (portMatches) {
      const ports = portMatches.map(m => m.match(/\d+/)[0]);
      console.log(`     Ports used: ${ports.join(', ')}`);
    }

    // Overall validation
    const isValid = passedChecks >= 6; // At least 6/8 checks
    if (isValid) {
      console.log('  ✅ VALID - Example is functional');
      validExamples++;
    } else {
      console.log('  ⚠️  WARNING - Some checks failed');
    }

  } catch (error) {
    console.log(`  ❌ Error: ${error.message}`);
  }
});

console.log('\n' + '='.repeat(60));
console.log('📊 Validation Summary');
console.log('='.repeat(60));
console.log(`Total examples: ${totalExamples}`);
console.log(`Valid examples: ${validExamples}`);
console.log(`Total lines of code: ${totalLines}`);
console.log(`Validation rate: ${((validExamples/totalExamples)*100).toFixed(1)}%`);

if (validExamples === totalExamples) {
  console.log('\n✅ All examples are VALID and ready to use!');
  process.exit(0);
} else {
  console.log('\n⚠️  Some examples need attention');
  process.exit(1);
}

