#!/usr/bin/env elixir

# Basic Envelope Example for UMICP Elixir SDK

Mix.install([{:jason, "~> 1.4"}])

Code.prepend_path("_build/dev/lib/umicp/ebin")

defmodule BasicEnvelopeExample do
  alias UMICP.Envelope

  def run do
    IO.puts("\n=== Basic Envelope Example ===\n")

    # Create a simple envelope
    {:ok, envelope} =
      Envelope.builder()
      |> Envelope.from("client-001")
      |> Envelope.to("server-001")
      |> Envelope.operation(:data)
      |> Envelope.message_id("msg-#{System.system_time()}")
      |> Envelope.capability("content-type", "application/json")
      |> Envelope.build_with_hash()

    IO.puts("Envelope created:")
    IO.puts("  From: #{envelope.from}")
    IO.puts("  To: #{envelope.to}")
    IO.puts("  Operation: #{envelope.operation}")
    IO.puts("  Message ID: #{envelope.message_id}")
    IO.puts("  Hash: #{envelope.hash}")

    # Serialize to JSON
    {:ok, json} = Envelope.serialize(envelope)
    IO.puts("\nSerialized JSON:")
    IO.puts(json)

    # Deserialize back
    {:ok, restored} = Envelope.deserialize(json)
    IO.puts("\nDeserialized envelope:")
    IO.puts("  From: #{restored.from}")
    IO.puts("  Hash verified: #{Envelope.verify_hash(restored)}")

    IO.puts("\n✅ Example completed!")
  end
end

BasicEnvelopeExample.run()

