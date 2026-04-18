/**
 * test_RequestFormatters.cpp – Comprehensive tests for RequestFormatters module.
 *
 * Covers:
 *   - formatOpenAIRequest  (model/prompt, optional params, gpt-5 token key)
 *   - formatOllamaRequest  (model/prompt, keepAlive, repeat_penalty)
 *   - formatClaudeRequest  (messages structure, system at top-level)
 *   - formatSimpleRequest  (basic structure)
 *   - getFormatterForEndpoint (routing to correct formatter)
 */

#include "test_framework.h"
#include "RequestFormatters.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Helper to parse the string result as JSON (throws on failure)
static json parseResult(const std::string &s)
{
    return json::parse(s);
}

// =========================================================================
TEST_SUITE("RequestFormatters::formatOpenAIRequest")
// =========================================================================

TEST_CASE("basic request contains model and user message")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Hello?", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));

    ASSERT_EQ(result["model"].get<std::string>(), "gpt-4");
    ASSERT_TRUE(result.contains("messages"));
    ASSERT_TRUE(result["messages"].is_array());
    ASSERT_EQ(result["messages"].size(), 1u);
    ASSERT_EQ(result["messages"][0]["role"].get<std::string>(), "user");
    ASSERT_EQ(result["messages"][0]["content"].get<std::string>(), "Hello?");
}

TEST_CASE("system prompt is prepended as a system message")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Prompt", L"You are helpful", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));

    ASSERT_EQ(result["messages"].size(), 2u);
    ASSERT_EQ(result["messages"][0]["role"].get<std::string>(), "system");
    ASSERT_EQ(result["messages"][0]["content"].get<std::string>(), "You are helpful");
    ASSERT_EQ(result["messages"][1]["role"].get<std::string>(), "user");
}

TEST_CASE("empty system prompt produces no system message")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Q", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_EQ(result["messages"].size(), 1u);
}

TEST_CASE("temperature 1.0 is omitted (default)")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Q", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_FALSE(result.contains("temperature"));
}

TEST_CASE("non-default temperature is included")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Q", L"", 0.7f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("temperature"));
    ASSERT_EQ(result["temperature"].get<float>(), 0.7f);
}

TEST_CASE("maxTokens = 0 is omitted")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Q", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_FALSE(result.contains("max_tokens"));
    ASSERT_FALSE(result.contains("max_completion_tokens"));
}

TEST_CASE("positive maxTokens produces max_tokens key for non-gpt-5 model")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Q", L"", 1.0f, 100, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("max_tokens"));
    ASSERT_EQ(result["max_tokens"].get<int>(), 100);
}

TEST_CASE("gpt-5 model uses max_completion_tokens instead of max_tokens")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-5", L"Q", L"", 1.0f, 200, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_FALSE(result.contains("max_tokens"));
    ASSERT_TRUE(result.contains("max_completion_tokens"));
    ASSERT_EQ(result["max_completion_tokens"].get<int>(), 200);
}

TEST_CASE("top_p 1.0 is omitted (default)")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Q", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_FALSE(result.contains("top_p"));
}

TEST_CASE("non-default top_p is included")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Q", L"", 1.0f, 0, 0.9f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("top_p"));
}

TEST_CASE("frequency_penalty 0.0 is omitted")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Q", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_FALSE(result.contains("frequency_penalty"));
}

TEST_CASE("non-zero frequency_penalty is included")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Q", L"", 1.0f, 0, 1.0f, 0.5f, 0.0f, L""));
    ASSERT_TRUE(result.contains("frequency_penalty"));
}

TEST_CASE("presence_penalty 0.0 is omitted")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Q", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_FALSE(result.contains("presence_penalty"));
}

TEST_CASE("non-zero presence_penalty is included")
{
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Q", L"", 1.0f, 0, 1.0f, 0.0f, 0.3f, L""));
    ASSERT_TRUE(result.contains("presence_penalty"));
}

TEST_CASE("keepAlive parameter is ignored for OpenAI requests")
{
    // keepAlive should not appear in the OpenAI payload
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"Q", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L"3600"));
    ASSERT_FALSE(result.contains("keep_alive"));
}

// =========================================================================
TEST_SUITE("RequestFormatters::formatOllamaRequest")
// =========================================================================

TEST_CASE("basic Ollama request has model and prompt at top level")
{
    auto result = parseResult(RequestFormatters::formatOllamaRequest(
        L"llama3", L"Tell me a joke", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));

    ASSERT_EQ(result["model"].get<std::string>(), "llama3");
    ASSERT_EQ(result["prompt"].get<std::string>(), "Tell me a joke");
    ASSERT_FALSE(result.contains("messages")); // Ollama doesn't use messages array
}

TEST_CASE("system prompt is included as system field")
{
    auto result = parseResult(RequestFormatters::formatOllamaRequest(
        L"llama3", L"Q", L"Be concise", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("system"));
    ASSERT_EQ(result["system"].get<std::string>(), "Be concise");
}

TEST_CASE("empty system prompt is omitted")
{
    auto result = parseResult(RequestFormatters::formatOllamaRequest(
        L"llama3", L"Q", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_FALSE(result.contains("system"));
}

TEST_CASE("keepAlive is included when non-empty")
{
    auto result = parseResult(RequestFormatters::formatOllamaRequest(
        L"llama3", L"Q", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L"10m"));
    ASSERT_TRUE(result.contains("keep_alive"));
    ASSERT_EQ(result["keep_alive"].get<std::string>(), "10m");
}

TEST_CASE("keepAlive = 0 is included (unload immediately)")
{
    auto result = parseResult(RequestFormatters::formatOllamaRequest(
        L"llama3", L"Q", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L"0"));
    ASSERT_TRUE(result.contains("keep_alive"));
    ASSERT_EQ(result["keep_alive"].get<std::string>(), "0");
}

TEST_CASE("empty keepAlive is omitted")
{
    auto result = parseResult(RequestFormatters::formatOllamaRequest(
        L"llama3", L"Q", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_FALSE(result.contains("keep_alive"));
}

TEST_CASE("maxTokens maps to num_predict for Ollama")
{
    auto result = parseResult(RequestFormatters::formatOllamaRequest(
        L"llama3", L"Q", L"", 1.0f, 512, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("num_predict"));
    ASSERT_EQ(result["num_predict"].get<int>(), 512);
    ASSERT_FALSE(result.contains("max_tokens"));
}

TEST_CASE("frequencyPenalty maps to repeat_penalty with offset of 1.0")
{
    auto result = parseResult(RequestFormatters::formatOllamaRequest(
        L"llama3", L"Q", L"", 1.0f, 0, 1.0f, 0.5f, 0.0f, L""));
    ASSERT_TRUE(result.contains("repeat_penalty"));
    float rp = result["repeat_penalty"].get<float>();
    // repeat_penalty = 1.0 + frequencyPenalty
    ASSERT_TRUE(rp > 1.4f && rp < 1.6f);
}

// =========================================================================
TEST_SUITE("RequestFormatters::formatClaudeRequest")
// =========================================================================

TEST_CASE("Claude request uses messages array with single user message")
{
    auto result = parseResult(RequestFormatters::formatClaudeRequest(
        L"claude-3-opus", L"Question", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));

    ASSERT_EQ(result["model"].get<std::string>(), "claude-3-opus");
    ASSERT_TRUE(result["messages"].is_array());
    ASSERT_EQ(result["messages"].size(), 1u);
    ASSERT_EQ(result["messages"][0]["role"].get<std::string>(), "user");
    ASSERT_EQ(result["messages"][0]["content"].get<std::string>(), "Question");
}

TEST_CASE("system prompt goes to top-level system field, not into messages")
{
    auto result = parseResult(RequestFormatters::formatClaudeRequest(
        L"claude-3-opus", L"Q", L"Be helpful", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));

    ASSERT_TRUE(result.contains("system"));
    ASSERT_EQ(result["system"].get<std::string>(), "Be helpful");
    // messages array should still have only the user message
    ASSERT_EQ(result["messages"].size(), 1u);
    ASSERT_EQ(result["messages"][0]["role"].get<std::string>(), "user");
}

TEST_CASE("empty system prompt is omitted from Claude request")
{
    auto result = parseResult(RequestFormatters::formatClaudeRequest(
        L"claude-3-opus", L"Q", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_FALSE(result.contains("system"));
}

TEST_CASE("Claude request includes max_tokens when provided")
{
    auto result = parseResult(RequestFormatters::formatClaudeRequest(
        L"claude-3-opus", L"Q", L"", 1.0f, 1024, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("max_tokens"));
    ASSERT_EQ(result["max_tokens"].get<int>(), 1024);
}

TEST_CASE("Claude request omits frequency and presence penalty")
{
    auto result = parseResult(RequestFormatters::formatClaudeRequest(
        L"claude-3-opus", L"Q", L"", 1.0f, 0, 1.0f, 0.9f, 0.9f, L""));
    ASSERT_FALSE(result.contains("frequency_penalty"));
    ASSERT_FALSE(result.contains("presence_penalty"));
}

// =========================================================================
TEST_SUITE("RequestFormatters::formatSimpleRequest")
// =========================================================================

TEST_CASE("simple request contains model and prompt")
{
    auto result = parseResult(RequestFormatters::formatSimpleRequest(
        L"mymodel", L"Prompt text", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));

    ASSERT_EQ(result["model"].get<std::string>(), "mymodel");
    ASSERT_EQ(result["prompt"].get<std::string>(), "Prompt text");
}

TEST_CASE("simple request includes system prompt when present")
{
    auto result = parseResult(RequestFormatters::formatSimpleRequest(
        L"mymodel", L"Q", L"System context", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("system"));
    ASSERT_EQ(result["system"].get<std::string>(), "System context");
}

TEST_CASE("simple request includes temperature when non-default")
{
    auto result = parseResult(RequestFormatters::formatSimpleRequest(
        L"mymodel", L"Q", L"", 0.5f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("temperature"));
}

TEST_CASE("simple request includes max_tokens when positive")
{
    auto result = parseResult(RequestFormatters::formatSimpleRequest(
        L"mymodel", L"Q", L"", 1.0f, 256, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("max_tokens"));
    ASSERT_EQ(result["max_tokens"].get<int>(), 256);
}

// =========================================================================
TEST_SUITE("RequestFormatters::getFormatterForEndpoint")
// =========================================================================

TEST_CASE("openai type routes to OpenAI formatter")
{
    auto fmt = RequestFormatters::getFormatterForEndpoint(L"openai");
    auto result = parseResult(fmt(L"gpt-4", L"Hello", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("messages"));
}

TEST_CASE("empty string routes to OpenAI formatter (default)")
{
    auto fmt = RequestFormatters::getFormatterForEndpoint(L"");
    auto result = parseResult(fmt(L"gpt-4", L"Hello", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("messages"));
}

TEST_CASE("ollama type routes to Ollama formatter")
{
    auto fmt = RequestFormatters::getFormatterForEndpoint(L"ollama");
    auto result = parseResult(fmt(L"llama3", L"Hi", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("prompt"));
    ASSERT_FALSE(result.contains("messages"));
}

TEST_CASE("claude type routes to Claude formatter")
{
    auto fmt = RequestFormatters::getFormatterForEndpoint(L"claude");
    auto result = parseResult(fmt(L"claude-3", L"Hi", L"Sys", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("messages"));
    ASSERT_TRUE(result.contains("system"));
}

TEST_CASE("simple type routes to simple formatter")
{
    auto fmt = RequestFormatters::getFormatterForEndpoint(L"simple");
    auto result = parseResult(fmt(L"mymodel", L"Hi", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("prompt"));
    ASSERT_FALSE(result.contains("messages"));
}

TEST_CASE("unknown type defaults to OpenAI formatter")
{
    auto fmt = RequestFormatters::getFormatterForEndpoint(L"unknownformat");
    auto result = parseResult(fmt(L"gpt-4", L"Hi", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    ASSERT_TRUE(result.contains("messages"));
}

// =========================================================================
TEST_SUITE("RequestFormatters – Unicode content handling")
// =========================================================================

TEST_CASE("UTF-8 encoded prompt is preserved in OpenAI request")
{
    // Japanese "Hello" in wide string
    auto result = parseResult(RequestFormatters::formatOpenAIRequest(
        L"gpt-4", L"\u3053\u3093\u306B\u3061\u306F", L"", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    std::string content = result["messages"][0]["content"].get<std::string>();
    // Content must be non-empty and start with a UTF-8 multibyte sequence
    ASSERT_FALSE(content.empty());
}

TEST_CASE("UTF-8 system prompt is preserved in Ollama request")
{
    auto result = parseResult(RequestFormatters::formatOllamaRequest(
        L"llama3", L"Q", L"\u4F60\u597D", 1.0f, 0, 1.0f, 0.0f, 0.0f, L""));
    std::string sys = result["system"].get<std::string>();
    ASSERT_FALSE(sys.empty());
}
