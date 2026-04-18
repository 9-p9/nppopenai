/**
 * test_ResponseParsers.cpp – Comprehensive tests for ResponseParsers module.
 *
 * Covers:
 *   - parseOpenAIResponse  (valid / edge / error paths)
 *   - parseOllamaResponse  (non-streamed / streamed / error paths)
 *   - parseClaudeResponse  (single and multi-part content / error paths)
 *   - parseSimpleResponse  (all known field names / error paths)
 *   - processThinkingSections (removal / preservation / edge cases)
 *   - getParserForEndpoint   (routing to correct parser)
 */

#include "test_framework.h"
#include "ResponseParsers.h"

// The global flag read by processThinkingSections lives in the stubs.
extern std::wstring configAPIValue_showReasoning;

// =========================================================================
TEST_SUITE("ResponseParsers::parseOpenAIResponse")
// =========================================================================

TEST_CASE("valid response returns content string")
{
    std::string json = R"({"choices":[{"message":{"content":"Hello world"}}]})";
    ASSERT_EQ(ResponseParsers::parseOpenAIResponse(json), "Hello world");
}

TEST_CASE("content with special characters is returned verbatim")
{
    std::string json = R"({"choices":[{"message":{"content":"line1\nline2\ttab"}}]})";
    std::string result = ResponseParsers::parseOpenAIResponse(json);
    ASSERT_CONTAINS(result, "line1\nline2");
}

TEST_CASE("empty choices array returns error message")
{
    std::string json = R"({"choices":[]})";
    std::string result = ResponseParsers::parseOpenAIResponse(json);
    ASSERT_CONTAINS(result, "[Error:");
}

TEST_CASE("missing choices field returns error message")
{
    std::string json = R"({"model":"gpt-4","usage":{}})";
    std::string result = ResponseParsers::parseOpenAIResponse(json);
    ASSERT_CONTAINS(result, "[Error:");
}

TEST_CASE("invalid JSON returns failure message")
{
    std::string json = "this is not json at all";
    std::string result = ResponseParsers::parseOpenAIResponse(json);
    ASSERT_CONTAINS(result, "[Failed to parse OpenAI response:");
}

TEST_CASE("empty string returns failure message")
{
    std::string result = ResponseParsers::parseOpenAIResponse("");
    ASSERT_CONTAINS(result, "[Failed to parse OpenAI response:");
}

TEST_CASE("nested JSON in content is returned as-is")
{
    // Content may itself contain JSON-like text
    std::string json = R"({"choices":[{"message":{"content":"{\"key\":\"val\"}"}}]})";
    std::string result = ResponseParsers::parseOpenAIResponse(json);
    ASSERT_CONTAINS(result, "key");
}

// =========================================================================
TEST_SUITE("ResponseParsers::parseOllamaResponse")
// =========================================================================

TEST_CASE("non-streamed valid response returns content")
{
    std::string json = R"({"response":"Ollama reply"})";
    ASSERT_EQ(ResponseParsers::parseOllamaResponse(json), "Ollama reply");
}

TEST_CASE("non-streamed error field returns formatted error")
{
    std::string json = R"({"error":"model not found"})";
    std::string result = ResponseParsers::parseOllamaResponse(json);
    ASSERT_CONTAINS(result, "[Error from Ollama:");
    ASSERT_CONTAINS(result, "model not found");
}

TEST_CASE("non-streamed missing response field returns descriptive error")
{
    std::string json = R"({"status":"done"})";
    std::string result = ResponseParsers::parseOllamaResponse(json);
    ASSERT_CONTAINS(result, "[Error:");
}

TEST_CASE("empty string returns error")
{
    std::string result = ResponseParsers::parseOllamaResponse("");
    ASSERT_CONTAINS(result, "[Error:");
}

TEST_CASE("streamed response (newline-separated) parses last chunk")
{
    std::string json = R"({"response":"part1"})" "\n"
                       R"({"response":"part2","done":false})" "\n"
                       R"({"response":"final","done":true})";
    std::string result = ResponseParsers::parseOllamaResponse(json);
    ASSERT_CONTAINS(result, "final");
}

TEST_CASE("invalid JSON returns failure message")
{
    std::string json = "broken{json}";
    std::string result = ResponseParsers::parseOllamaResponse(json);
    ASSERT_CONTAINS(result, "[Failed to parse Ollama response:");
}

TEST_CASE("streamed response with no valid JSON in last chunk returns error")
{
    std::string json = "line1\nline2\nno_brace_here";
    std::string result = ResponseParsers::parseOllamaResponse(json);
    // Either the parse fails or returns an error about invalid format
    ASSERT_CONTAINS(result, "[Error");
}

// =========================================================================
TEST_SUITE("ResponseParsers::parseClaudeResponse")
// =========================================================================

TEST_CASE("valid single text block returns content")
{
    std::string json = R"({"content":[{"type":"text","text":"Claude reply"}]})";
    ASSERT_EQ(ResponseParsers::parseClaudeResponse(json), "Claude reply");
}

TEST_CASE("multiple text blocks are concatenated")
{
    std::string json = R"({"content":[{"type":"text","text":"Hello "},{"type":"text","text":"world"}]})";
    std::string result = ResponseParsers::parseClaudeResponse(json);
    ASSERT_CONTAINS(result, "Hello ");
    ASSERT_CONTAINS(result, "world");
}

TEST_CASE("non-text content types are skipped")
{
    std::string json = R"({"content":[{"type":"image","source":{}},{"type":"text","text":"answer"}]})";
    std::string result = ResponseParsers::parseClaudeResponse(json);
    ASSERT_EQ(result, "answer");
}

TEST_CASE("empty content array returns error")
{
    std::string json = R"({"content":[]})";
    std::string result = ResponseParsers::parseClaudeResponse(json);
    ASSERT_CONTAINS(result, "[Error:");
}

TEST_CASE("content array with no text-type blocks returns error")
{
    std::string json = R"({"content":[{"type":"tool_use","id":"x"}]})";
    std::string result = ResponseParsers::parseClaudeResponse(json);
    ASSERT_CONTAINS(result, "[Error:");
}

TEST_CASE("missing content field returns error")
{
    std::string json = R"({"id":"msg_01"})";
    std::string result = ResponseParsers::parseClaudeResponse(json);
    ASSERT_CONTAINS(result, "[Error:");
}

TEST_CASE("invalid JSON returns failure message")
{
    std::string result = ResponseParsers::parseClaudeResponse("not json");
    ASSERT_CONTAINS(result, "[Failed to parse Claude response:");
}

// =========================================================================
TEST_SUITE("ResponseParsers::parseSimpleResponse")
// =========================================================================

TEST_CASE("text field is returned")
{
    std::string json = R"({"text":"simple reply"})";
    ASSERT_EQ(ResponseParsers::parseSimpleResponse(json), "simple reply");
}

TEST_CASE("completion field is returned when text is absent")
{
    std::string json = R"({"completion":"done"})";
    ASSERT_EQ(ResponseParsers::parseSimpleResponse(json), "done");
}

TEST_CASE("output field is returned when text and completion are absent")
{
    std::string json = R"({"output":"result"})";
    ASSERT_EQ(ResponseParsers::parseSimpleResponse(json), "result");
}

TEST_CASE("generated_text field is returned as fallback")
{
    std::string json = R"({"generated_text":"generated"})";
    ASSERT_EQ(ResponseParsers::parseSimpleResponse(json), "generated");
}

TEST_CASE("no recognised field returns error")
{
    std::string json = R"({"response":"x","answer":"y"})";
    std::string result = ResponseParsers::parseSimpleResponse(json);
    ASSERT_CONTAINS(result, "[Error:");
}

TEST_CASE("invalid JSON returns failure message")
{
    std::string result = ResponseParsers::parseSimpleResponse("{bad json");
    ASSERT_CONTAINS(result, "[Failed to parse simple response:");
}

// =========================================================================
TEST_SUITE("ResponseParsers::processThinkingSections")
// =========================================================================

TEST_CASE("single think section is removed when showReasoning is 0")
{
    configAPIValue_showReasoning = L"0";
    std::string input = "Before<think>internal reasoning</think>After";
    std::string result = ResponseParsers::processThinkingSections(input);
    ASSERT_EQ(result, "BeforeAfter");
    ASSERT_NOT_CONTAINS(result, "internal reasoning");
}

TEST_CASE("multiple think sections are all removed")
{
    configAPIValue_showReasoning = L"0";
    std::string input = "A<think>r1</think>B<think>r2</think>C";
    std::string result = ResponseParsers::processThinkingSections(input);
    ASSERT_EQ(result, "ABC");
}

TEST_CASE("think sections are preserved when showReasoning is 1")
{
    configAPIValue_showReasoning = L"1";
    std::string input = "Hello<think>reasoning</think>World";
    std::string result = ResponseParsers::processThinkingSections(input);
    ASSERT_EQ(result, input);
}

TEST_CASE("text without think tags is returned unchanged")
{
    configAPIValue_showReasoning = L"0";
    std::string input = "No thinking here, just plain text.";
    ASSERT_EQ(ResponseParsers::processThinkingSections(input), input);
}

TEST_CASE("unclosed think tag does not cause infinite loop and returns partial text")
{
    configAPIValue_showReasoning = L"0";
    std::string input = "Start<think>unclosed";
    // Should return everything up to the opening tag (loop breaks on missing </think>)
    std::string result = ResponseParsers::processThinkingSections(input);
    ASSERT_CONTAINS(result, "Start");
}

TEST_CASE("empty string is handled")
{
    configAPIValue_showReasoning = L"0";
    ASSERT_EQ(ResponseParsers::processThinkingSections(""), "");
}

TEST_CASE("adjacent think sections are all removed")
{
    configAPIValue_showReasoning = L"0";
    std::string input = "<think>a</think><think>b</think>Final";
    std::string result = ResponseParsers::processThinkingSections(input);
    ASSERT_EQ(result, "Final");
}

// =========================================================================
TEST_SUITE("ResponseParsers::getParserForEndpoint")
// =========================================================================

TEST_CASE("openai type routes to OpenAI parser")
{
    auto parser = ResponseParsers::getParserForEndpoint(L"openai");
    std::string json = R"({"choices":[{"message":{"content":"hi"}}]})";
    ASSERT_EQ(parser(json), "hi");
}

TEST_CASE("empty string routes to OpenAI parser (default)")
{
    auto parser = ResponseParsers::getParserForEndpoint(L"");
    std::string json = R"({"choices":[{"message":{"content":"default"}}]})";
    ASSERT_EQ(parser(json), "default");
}

TEST_CASE("ollama type routes to Ollama parser")
{
    auto parser = ResponseParsers::getParserForEndpoint(L"ollama");
    std::string json = R"({"response":"ollama-reply"})";
    ASSERT_EQ(parser(json), "ollama-reply");
}

TEST_CASE("claude type routes to Claude parser")
{
    auto parser = ResponseParsers::getParserForEndpoint(L"claude");
    std::string json = R"({"content":[{"type":"text","text":"claude-reply"}]})";
    ASSERT_EQ(parser(json), "claude-reply");
}

TEST_CASE("simple type routes to simple parser")
{
    auto parser = ResponseParsers::getParserForEndpoint(L"simple");
    std::string json = R"({"text":"simple-reply"})";
    ASSERT_EQ(parser(json), "simple-reply");
}

TEST_CASE("unknown type defaults to OpenAI parser")
{
    auto parser = ResponseParsers::getParserForEndpoint(L"unknownformat");
    std::string json = R"({"choices":[{"message":{"content":"fallback"}}]})";
    ASSERT_EQ(parser(json), "fallback");
}

// =========================================================================
TEST_SUITE("ResponseParsers – think sections inside real API responses")
// =========================================================================

TEST_CASE("OpenAI response with think section – section removed by default")
{
    configAPIValue_showReasoning = L"0";
    std::string json = R"({"choices":[{"message":{"content":"<think>internal</think>Answer"}}]})";
    std::string result = ResponseParsers::parseOpenAIResponse(json);
    ASSERT_EQ(result, "Answer");
}

TEST_CASE("Ollama response with think section – section removed by default")
{
    configAPIValue_showReasoning = L"0";
    std::string json = R"({"response":"<think>thinking</think>Reply"})";
    std::string result = ResponseParsers::parseOllamaResponse(json);
    ASSERT_EQ(result, "Reply");
}
