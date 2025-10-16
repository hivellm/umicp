defmodule UMICP.Transport.HTTP2 do
  @moduledoc """
  HTTP/2 transport for UMICP using Finch.
  """

  alias UMICP.Envelope

  @doc """
  Send an envelope via HTTP/2 POST.
  """
  @spec send(String.t(), Envelope.t(), keyword()) ::
          {:ok, Envelope.t()} | {:error, term()}
  def send(base_url, envelope, opts \\ []) do
    endpoint = Keyword.get(opts, :endpoint, "/umicp")
    url = base_url <> endpoint

    with {:ok, json} <- Envelope.serialize(envelope),
         {:ok, %Finch.Response{status: status, body: body}} when status in 200..299 <-
           Finch.build(:post, url, [{"content-type", "application/json"}], json)
           |> Finch.request(UMICPFinch),
         {:ok, response_envelope} <- Envelope.deserialize(body) do
      {:ok, response_envelope}
    else
      {:ok, %Finch.Response{status: status}} ->
        {:error, {:http_error, status}}

      {:error, reason} ->
        {:error, reason}
    end
  end

  @doc """
  Send GET request and receive envelope.
  """
  @spec get(String.t(), String.t()) :: {:ok, Envelope.t()} | {:error, term()}
  def get(base_url, endpoint) do
    url = base_url <> endpoint

    with {:ok, %Finch.Response{status: status, body: body}} when status in 200..299 <-
           Finch.build(:get, url)
           |> Finch.request(UMICPFinch),
         {:ok, envelope} <- Envelope.deserialize(body) do
      {:ok, envelope}
    else
      {:ok, %Finch.Response{status: status}} ->
        {:error, {:http_error, status}}

      {:error, reason} ->
        {:error, reason}
    end
  end
end

