/**
 * test_StreamParser.cpp – Comprehensive tests for StreamParser module.
 *
 * Covers:
 *   - isCompletionMarker   (all recognised DONE variants)
 *   - parseOpenAIChunk     (SSE data: lines, multi-event, DONE)
 *   - parseOllamaChunk     (JSON response / done / error)
 *   - parseClaudeChunk     (content_block_delta / other types)
 *   - extractContent       (type dispatch and raw JSON fast-path)
 */

#include "test_framework.h"
#include "StreamParser.h"

extern bool debugMode;   // from globals_stub – keep false to avoid file I/O

// =========================================================================
TEST_SUITE("StreamParser::isCompletionMarker")
// =========================================================================

TEST_CASE("bare [DONE] string is a marker")
{
    ASSERT_TRUE(StreamParser::isCompletionMarker("data: [DONE]"));
}

TEST_CASE("[DONE] with LF is a marker")
{
    ASSERT_TRUE(StreamParser::isCompletionMarker("data: [DONE]\n"));
}

TEST_CASE("[DONE] with CRLF is a marker")
{
    ASSERT_TRUE(StreamParser::isCompletionMarker("data: [DONE]\r\n"));
}

TEST_CASE("string starting with data: [DONE] is a marker")
{
    ASSERT_TRUE(StreamParser::isCompletionMarker("data: [DONE] trailing"));
}

TEST_CASE("regular data line is not a marker")
{
    ASSERT_FALSE(StreamParser::isCompletionMarker("data: {\"choices\":[]}"));
}

TEST_CASE("empty string is not a marker")
{
    ASSERT_FALSE(StreamParser::isCompletionMarker(""));
}

TEST_CASE("random text is not a marker")
{
    ASSERT_FALSE(StreamParser::isCompletionMarker("Hello world"));
}

// =========================================================================
TEST_SUITE("StreamParser::parseOpenAIChunk")
// =========================================================================

TEST_CASE("single valid SSE line extracts delta content")
{
    std::string chunk = "data: {\"choices\":[{\"delta\":{\"content\":\"Hello\"}}]}\n";
    ASSERT_EQ(StreamParser::parseOpenAIChunk(chunk), "Hello");
}

TEST_CASE("multiple SSE events are concatenated")
{
    std::string chunk =
        "data: {\"choices\":[{\"delta\":{\"content\":\"Hel\"}}]}\n"
        "data: {\"choices\":[{\"delta\":{\"content\":\"lo\"}}]}\n";
    ASSERT_EQ(StreamParser::parseOpenAIChunk(chunk), "Hello");
}

TEST_CASE("[DONE] line is skipped and does not produce content")
{
    std::string chunk =
        "data: {\"choices\":[{\"delta\":{\"content\":\"Hi\"}}]}\n"
        "data: [DONE]\n";
    ASSERT_EQ(StreamParser::parseOpenAIChunk(chunk), "Hi");
}

TEST_CASE("empty chunk returns empty string")
{
    ASSERT_EQ(StreamParser::parseOpenAIChunk(""), "");
}

TEST_CASE("data line with invalid JSON is silently skipped")
{
    std::string chunk = "data: not-valid-json\n";
    ASSERT_EQ(StreamParser::parseOpenAIChunk(chunk), "");
}

TEST_CASE("blank lines between events are ignored")
{
    std::string chunk =
        "\r\n"
        "data: {\"choices\":[{\"delta\":{\"content\":\"A\"}}]}\n"
        "\r\n"
        "data: {\"choices\":[{\"delta\":{\"content\":\"B\"}}]}\n";
    ASSERT_EQ(StreamParser::parseOpenAIChunk(chunk), "AB");
}

TEST_CASE("delta without content key produces no output")
{
    std::string chunk = "data: {\"choices\":[{\"delta\":{\"role\":\"assistant\"}}]}\n";
    ASSERT_EQ(StreamParser::parseOpenAIChunk(chunk), "");
}

TEST_CASE("CRLF line endings are handled")
{
    std::string chunk = "data: {\"choices\":[{\"delta\":{\"content\":\"X\"}}]}\r\n";
    ASSERT_EQ(StreamParser::parseOpenAIChunk(chunk), "X");
}

// =========================================================================
TEST_SUITE("StreamParser::parseOllamaChunk")
// =========================================================================

TEST_CASE("valid chunk with response field returns content")
{
    std::string chunk = R"({"response":"word"})";
    ASSERT_EQ(StreamParser::parseOllamaChunk(chunk), "word");
}

TEST_CASE("done=true chunk with no response returns empty string")
{
    std::string chunk = R"({"done":true,"total_duration":1234})";
    ASSERT_EQ(StreamParser::parseOllamaChunk(chunk), "");
}

TEST_CASE("invalid JSON returns empty string")
{
    ASSERT_EQ(StreamParser::parseOllamaChunk("not json"), "");
}

TEST_CASE("empty string returns empty string")
{
    ASSERT_EQ(StreamParser::parseOllamaChunk(""), "");
}

TEST_CASE("chunk with response=empty string returns empty string")
{
    std::string chunk = R"({"response":""})";
    ASSERT_EQ(StreamParser::parseOllamaChunk(chunk), "");
}

// =========================================================================
TEST_SUITE("StreamParser::parseClaudeChunk")
// =========================================================================

TEST_CASE("content_block_delta with text returns the text")
{
    std::string chunk = R"({"type":"content_block_delta","index":0,"delta":{"type":"text_delta","text":"Hi"}})";
    ASSERT_EQ(StreamParser::parseClaudeChunk(chunk), "Hi");
}

TEST_CASE("wrong event type returns empty string")
{
    std::string chunk = R"({"type":"message_start","message":{}})";
    ASSERT_EQ(StreamParser::parseClaudeChunk(chunk), "");
}

TEST_CASE("content_block_delta without text key returns empty string")
{
    std::string chunk = R"({"type":"content_block_delta","delta":{"type":"other"}})";
    ASSERT_EQ(StreamParser::parseClaudeChunk(chunk), "");
}

TEST_CASE("invalid JSON returns empty string")
{
    ASSERT_EQ(StreamParser::parseClaudeChunk("{bad}"), "");
}

TEST_CASE("empty string returns empty string")
{
    ASSERT_EQ(StreamParser::parseClaudeChunk(""), "");
}

// =========================================================================
TEST_SUITE("StreamParser::extractContent")
// =========================================================================

TEST_CASE("empty chunk always returns empty string")
{
    ASSERT_EQ(StreamParser::extractContent("", "openai"), "");
    ASSERT_EQ(StreamParser::extractContent("", "ollama"), "");
    ASSERT_EQ(StreamParser::extractContent("", "claude"), "");
}

TEST_CASE("raw OpenAI JSON is extracted via fast-path (no data: prefix)")
{
    // extractContent tries JSON parse first regardless of apiType
    std::string chunk = R"({"choices":[{"delta":{"content":"fast"}}]})";
    ASSERT_EQ(StreamParser::extractContent(chunk, "openai"), "fast");
}

TEST_CASE("raw Ollama JSON is extracted via fast-path")
{
    std::string chunk = R"({"response":"ollama-stream"})";
    ASSERT_EQ(StreamParser::extractContent(chunk, "ollama"), "ollama-stream");
}

TEST_CASE("raw Claude JSON is extracted via fast-path")
{
    std::string chunk = R"({"type":"content_block_delta","delta":{"text":"claude-stream"}})";
    ASSERT_EQ(StreamParser::extractContent(chunk, "claude"), "claude-stream");
}

TEST_CASE("SSE openai chunk is parsed via type-specific fallback")
{
    std::string chunk = "data: {\"choices\":[{\"delta\":{\"content\":\"fallback\"}}]}\n";
    ASSERT_EQ(StreamParser::extractContent(chunk, "openai"), "fallback");
}

TEST_CASE("unknown JSON structure with openai type returns empty")
{
    std::string chunk = R"({"foo":"bar"})";
    ASSERT_EQ(StreamParser::extractContent(chunk, "openai"), "");
}

TEST_CASE("simple/empty apiType returns small raw chunks unchanged")
{
    // Small chunk (<100 chars) that is not a completion marker
    std::string chunk = "short text";
    // extractContent for simple: returns raw chunk if small and not [DONE]
    std::string result = StreamParser::extractContent(chunk, "simple");
    ASSERT_EQ(result, chunk);
}

TEST_CASE("simple apiType does not return [DONE] marker as content")
{
    std::string chunk = "data: [DONE]";
    std::string result = StreamParser::extractContent(chunk, "simple");
    // isCompletionMarker returns true -> should not return raw chunk
    ASSERT_EQ(result, "");
}
