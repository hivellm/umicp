defmodule UMICP.TypesTest do
  use ExUnit.Case, async: true

  alias UMICP.OperationType

  describe "OperationType.to_int/1" do
    test "converts all operation types to integers" do
      assert OperationType.to_int(:data) == 0
      assert OperationType.to_int(:control) == 1
      assert OperationType.to_int(:ack) == 2
      assert OperationType.to_int(:error) == 3
      assert OperationType.to_int(:request) == 4
      assert OperationType.to_int(:response) == 5
    end
  end

  describe "OperationType.from_int/1" do
    test "converts integers to operation types" do
      assert OperationType.from_int(0) == {:ok, :data}
      assert OperationType.from_int(1) == {:ok, :control}
      assert OperationType.from_int(2) == {:ok, :ack}
      assert OperationType.from_int(3) == {:ok, :error}
      assert OperationType.from_int(4) == {:ok, :request}
      assert OperationType.from_int(5) == {:ok, :response}
    end

    test "returns error for invalid integer" do
      assert OperationType.from_int(99) == {:error, :invalid_operation_type}
    end
  end

  describe "OperationType.all/0" do
    test "returns all operation types" do
      all = OperationType.all()

      assert length(all) == 6
      assert :data in all
      assert :control in all
      assert :ack in all
      assert :error in all
      assert :request in all
      assert :response in all
    end
  end

  describe "UMICP.Error" do
    test "creates error with type and message" do
      error = UMICP.Error.new(:validation_error, "Test error")

      assert error.type == :validation_error
      assert error.message == "Test error"
    end

    test "invalid_envelope/1 creates correct error" do
      error = UMICP.Error.invalid_envelope("Bad envelope")

      assert error.type == :invalid_envelope
      assert error.message == "Bad envelope"
    end

    test "validation_error/1 creates correct error" do
      error = UMICP.Error.validation_error("Validation failed")

      assert error.type == :validation_error
      assert error.message == "Validation failed"
    end
  end
end

