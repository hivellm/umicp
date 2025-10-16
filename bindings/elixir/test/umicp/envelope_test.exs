defmodule UMICP.EnvelopeTest do
  use ExUnit.Case, async: true

  alias UMICP.Envelope

  describe "new/1" do
    test "creates envelope with required fields" do
      envelope = Envelope.new(
        from: "client",
        to: "server",
        operation: :data,
        message_id: "msg-001"
      )

      assert envelope.from == "client"
      assert envelope.to == "server"
      assert envelope.operation == :data
      assert envelope.message_id == "msg-001"
      assert envelope.capabilities == %{}
    end

    test "creates envelope with capabilities" do
      envelope = Envelope.new(
        from: "client",
        to: "server",
        operation: :data,
        message_id: "msg-001",
        capabilities: %{"key" => "value"}
      )

      assert envelope.capabilities == %{"key" => "value"}
    end
  end

  describe "calculate_hash/1" do
    test "calculates SHA-256 hash" do
      envelope = Envelope.new(
        from: "client",
        to: "server",
        operation: :data,
        message_id: "msg-001"
      )

      envelope_with_hash = Envelope.calculate_hash(envelope)

      assert envelope_with_hash.hash != nil
      assert String.length(envelope_with_hash.hash) == 64
    end

    test "hash is consistent" do
      envelope = Envelope.new(
        from: "client",
        to: "server",
        operation: :data,
        message_id: "msg-001"
      )

      hash1 = Envelope.calculate_hash(envelope).hash
      hash2 = Envelope.calculate_hash(envelope).hash

      assert hash1 == hash2
    end
  end

  describe "validate/1" do
    test "validates correct envelope" do
      envelope = Envelope.new(
        from: "client",
        to: "server",
        operation: :data,
        message_id: "msg-001"
      )

      assert Envelope.validate(envelope) == :ok
    end

    test "rejects empty from" do
      envelope = Envelope.new(
        from: "",
        to: "server",
        operation: :data,
        message_id: "msg-001"
      )

      assert {:error, %UMICP.Error{type: :validation_error}} = Envelope.validate(envelope)
    end

    test "rejects empty to" do
      envelope = Envelope.new(
        from: "client",
        to: "",
        operation: :data,
        message_id: "msg-001"
      )

      assert {:error, %UMICP.Error{type: :validation_error}} = Envelope.validate(envelope)
    end

    test "rejects empty message_id" do
      envelope = Envelope.new(
        from: "client",
        to: "server",
        operation: :data,
        message_id: ""
      )

      assert {:error, %UMICP.Error{type: :validation_error}} = Envelope.validate(envelope)
    end
  end

  describe "verify_hash/1" do
    test "verifies correct hash" do
      envelope =
        Envelope.new(
          from: "client",
          to: "server",
          operation: :data,
          message_id: "msg-001"
        )
        |> Envelope.calculate_hash()

      assert Envelope.verify_hash(envelope) == true
    end

    test "rejects invalid hash" do
      envelope = Envelope.new(
        from: "client",
        to: "server",
        operation: :data,
        message_id: "msg-001",
        hash: "invalid"
      )

      assert Envelope.verify_hash(envelope) == false
    end
  end

  describe "serialize/1 and deserialize/1" do
    test "round-trip serialization" do
      original = Envelope.new(
        from: "client",
        to: "server",
        operation: :data,
        message_id: "msg-001",
        capabilities: %{"key" => "value"}
      )

      assert {:ok, json} = Envelope.serialize(original)
      assert {:ok, restored} = Envelope.deserialize(json)

      assert restored.from == original.from
      assert restored.to == original.to
      assert restored.message_id == original.message_id
    end
  end

  describe "capabilities" do
    test "get_capability/2 retrieves value" do
      envelope = Envelope.new(
        from: "client",
        to: "server",
        operation: :data,
        message_id: "msg-001",
        capabilities: %{"key" => "value"}
      )

      assert Envelope.get_capability(envelope, "key") == "value"
      assert Envelope.get_capability(envelope, "missing") == nil
    end

    test "set_capability/3 sets value" do
      envelope = Envelope.new(
        from: "client",
        to: "server",
        operation: :data,
        message_id: "msg-001"
      )

      updated = Envelope.set_capability(envelope, "key", "value")

      assert Envelope.get_capability(updated, "key") == "value"
    end

    test "supports native types in capabilities" do
      envelope = Envelope.new(
        from: "client",
        to: "server",
        operation: :data,
        message_id: "msg-001",
        capabilities: %{
          "string" => "text",
          "integer" => 42,
          "boolean" => true,
          "float" => 3.14,
          "array" => [1, 2, 3],
          "map" => %{"nested" => "value"}
        }
      )

      assert Envelope.get_capability(envelope, "string") == "text"
      assert Envelope.get_capability(envelope, "integer") == 42
      assert Envelope.get_capability(envelope, "boolean") == true
      assert Envelope.get_capability(envelope, "float") == 3.14
      assert Envelope.get_capability(envelope, "array") == [1, 2, 3]
      assert Envelope.get_capability(envelope, "map") == %{"nested" => "value"}
    end
  end

  describe "builder/0" do
    test "builds envelope with pipe syntax" do
      {:ok, envelope} =
        Envelope.builder()
        |> Envelope.from("client")
        |> Envelope.to("server")
        |> Envelope.operation(:data)
        |> Envelope.message_id("msg-001")
        |> Envelope.capability("key", "value")
        |> Envelope.build()

      assert envelope.from == "client"
      assert envelope.to == "server"
      assert envelope.operation == :data
      assert envelope.message_id == "msg-001"
      assert Envelope.get_capability(envelope, "key") == "value"
    end

    test "build_with_hash/1 includes hash" do
      {:ok, envelope} =
        Envelope.builder()
        |> Envelope.from("client")
        |> Envelope.to("server")
        |> Envelope.operation(:data)
        |> Envelope.message_id("msg-001")
        |> Envelope.build_with_hash()

      assert envelope.hash != nil
      assert Envelope.verify_hash(envelope) == true
    end
  end
end

