defmodule UMICP.MixProject do
  use Mix.Project

  @version "0.2.0"
  @source_url "https://github.com/hivellm/umicp"

  def project do
    [
      app: :umicp,
      version: @version,
      elixir: "~> 1.14",
      start_permanent: Mix.env() == :prod,
      deps: deps(),
      description: description(),
      package: package(),
      docs: docs(),
      name: "UMICP",
      source_url: @source_url,
      homepage_url: @source_url,
      test_coverage: [tool: ExCoveralls],
      preferred_cli_env: [
        coveralls: :test,
        "coveralls.detail": :test,
        "coveralls.post": :test,
        "coveralls.html": :test
      ]
    ]
  end

  def application do
    [
      extra_applications: [:logger, :crypto]
    ]
  end

  defp deps do
    [
      # JSON encoding/decoding
      {:jason, "~> 1.4"},

      # HTTP/2 and WebSocket support
      {:mint, "~> 1.5"},
      {:mint_web_socket, "~> 1.0"},
      {:finch, "~> 0.18"},

      # Optional: Numerical Elixir for matrix operations (disabled for testing with Elixir 1.14)
      # {:nx, "~> 0.7", optional: true},

      # Development and testing
      {:ex_doc, "~> 0.31", only: :dev, runtime: false},
      {:excoveralls, "~> 0.18", only: :test},
      {:credo, "~> 1.7", only: [:dev, :test], runtime: false},
      {:dialyxir, "~> 1.4", only: [:dev, :test], runtime: false}
    ]
  end

  defp description do
    """
    UMICP Elixir bindings for high-performance communication and matrix operations
    with native type support and MCP-compatible tool discovery.
    """
  end

  defp package do
    [
      name: "umicp",
      files: ~w(lib .formatter.exs mix.exs README.md LICENSE CHANGELOG.md),
      licenses: ["MIT"],
      links: %{
        "GitHub" => @source_url,
        "Documentation" => "https://hexdocs.pm/umicp"
      },
      maintainers: ["HiveLLM AI Collaborative Team"]
    ]
  end

  defp docs do
    [
      main: "UMICP",
      extras: ["README.md", "CHANGELOG.md"],
      source_ref: "v#{@version}",
      source_url: @source_url,
      formatters: ["html"]
    ]
  end
end

