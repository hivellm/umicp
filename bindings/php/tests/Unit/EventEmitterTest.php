<?php

namespace UMICP\Tests\Unit;

use PHPUnit\Framework\TestCase;
use UMICP\Core\EventEmitter;

class EventEmitterTest extends TestCase
{
    public function testOnAndEmit(): void
    {
        $emitter = new EventEmitter();
        $called = false;

        $emitter->on('test', function() use (&$called) {
            $called = true;
        });

        $emitter->emit('test');
        $this->assertTrue($called);
    }

    public function testMultipleListeners(): void
    {
        $emitter = new EventEmitter();
        $count = 0;

        $emitter->on('test', function() use (&$count) { $count++; });
        $emitter->on('test', function() use (&$count) { $count++; });
        $emitter->on('test', function() use (&$count) { $count++; });

        $emitter->emit('test');
        $this->assertEquals(3, $count);
    }

    public function testOnceListener(): void
    {
        $emitter = new EventEmitter();
        $count = 0;

        $emitter->once('test', function() use (&$count) {
            $count++;
        });

        $emitter->emit('test');
        $emitter->emit('test');
        $emitter->emit('test');

        $this->assertEquals(1, $count); // Should only fire once
    }

    public function testEmitWithArguments(): void
    {
        $emitter = new EventEmitter();
        $receivedArgs = [];

        $emitter->on('test', function(...$args) use (&$receivedArgs) {
            $receivedArgs = $args;
        });

        $emitter->emit('test', 'arg1', 'arg2', 123);

        $this->assertEquals(['arg1', 'arg2', 123], $receivedArgs);
    }

    public function testRemoveListener(): void
    {
        $emitter = new EventEmitter();
        $called = false;

        $listener = function() use (&$called) {
            $called = true;
        };

        $emitter->on('test', $listener);
        $emitter->off('test', $listener);
        $emitter->emit('test');

        $this->assertFalse($called);
    }

    public function testRemoveAllListeners(): void
    {
        $emitter = new EventEmitter();
        $count = 0;

        $emitter->on('test1', function() use (&$count) { $count++; });
        $emitter->on('test2', function() use (&$count) { $count++; });

        $emitter->removeAllListeners();
        $emitter->emit('test1');
        $emitter->emit('test2');

        $this->assertEquals(0, $count);
    }

    public function testListenerCount(): void
    {
        $emitter = new EventEmitter();

        $this->assertEquals(0, $emitter->listenerCount('test'));

        $emitter->on('test', function() {});
        $this->assertEquals(1, $emitter->listenerCount('test'));

        $emitter->on('test', function() {});
        $this->assertEquals(2, $emitter->listenerCount('test'));

        $emitter->once('test', function() {});
        $this->assertEquals(3, $emitter->listenerCount('test'));
    }

    public function testEventNames(): void
    {
        $emitter = new EventEmitter();

        $emitter->on('event1', function() {});
        $emitter->on('event2', function() {});
        $emitter->once('event3', function() {});

        $names = $emitter->eventNames();
        $this->assertCount(3, $names);
        $this->assertContains('event1', $names);
        $this->assertContains('event2', $names);
        $this->assertContains('event3', $names);
    }

    public function testGetStats(): void
    {
        $emitter = new EventEmitter();

        $emitter->on('test', function() {});
        $emitter->on('test', function() {});
        $emitter->emit('test');
        $emitter->emit('test');

        $stats = $emitter->getStats();
        $this->assertArrayHasKey('test', $stats);
        $this->assertEquals(2, $stats['test']['emits']);
        $this->assertEquals(2, $stats['test']['listeners']);
    }

    public function testMethodChaining(): void
    {
        $emitter = new EventEmitter();

        $result = $emitter
            ->on('test1', function() {})
            ->on('test2', function() {})
            ->once('test3', function() {})
            ->emit('test1')
            ->off('test2');

        $this->assertInstanceOf(EventEmitter::class, $result);
    }
}

