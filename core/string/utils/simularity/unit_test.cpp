#include "levenshtein_distance.hpp"
#include "ngram.hpp"
#include "simhash.hpp"

using namespace ymd;
using namespace ymd::str;

namespace{

[[maybe_unused]] void test_levenshtein_distance(){
    {
        constexpr auto str1 = StringView("kitten");
        constexpr auto str2 = StringView("sitting");
        constexpr auto distance = levenshtein_distance(str1, str2);
        static_assert(distance == 3);
    }

    {
        constexpr auto str1 = StringView("");
        constexpr auto str2 = StringView("");
        constexpr auto distance = levenshtein_distance(str1, str2);
        static_assert(distance == 0);
    }

    {
        constexpr auto str1 = StringView("");
        constexpr auto str2 = StringView("sitting");
        constexpr auto distance = levenshtein_distance(str1, str2);
        static_assert(distance == 7);
    }

    {
        constexpr auto str1 = StringView("sitting");
        constexpr auto str2 = StringView("");
        constexpr auto distance = levenshtein_distance(str1, str2);
        static_assert(distance == 7);
    }

    {
        constexpr auto str1 = StringView("apple");
        constexpr auto str2 = StringView("penapple");
        constexpr auto distance = levenshtein_distance(str1, str2);
        static_assert(distance == 3);
    }
}

[[maybe_unused]] void test_ngram_similarity(){
    {
        constexpr auto str1 = StringView("");
        constexpr auto str2 = StringView("");
        constexpr auto distance = ngram_similarity(str1, str2);
        static_assert(distance == 1.0f);
    }

    {
        constexpr auto str1 = StringView("apple");
        constexpr auto str2 = StringView("penapple");
        constexpr auto distance = ngram_similarity(str1, str2);
        static_assert(distance > 0.7);
    }

    {
        constexpr auto similarity = ngram_similarity("add", "add2");
        constexpr auto similarity2 = ngram_similarity("add", "adopt");
        static_assert(similarity > similarity2);
    }

    {
        constexpr auto similarity = ngram_similarity("i love rusty", "i love rust");
        constexpr auto similarity2 = ngram_similarity("i love code", "i love cpp");
        static_assert(similarity > similarity2);
    }
}

[[maybe_unused]] void test_simhash(){
    {
        constexpr auto str1 = StringView("apple");
        constexpr auto str2 = StringView("apple");
        constexpr auto similarity = simhash_similarity(str1, str2);
        static_assert(similarity == 1);
    }

    {
        constexpr auto str1 = StringView("apple");
        constexpr auto str2 = StringView("penapple");
        constexpr auto similarity = simhash_similarity(str1, str2);
        static_assert(similarity > 0.9f);
    }

    {
        constexpr auto str1 = StringView("pig");
        constexpr auto str2 = StringView("duck");
        constexpr auto similarity = simhash_similarity(str1, str2);
        static_assert(similarity < 0.9f);
    }

    {
        constexpr auto similarity = simhash_similarity("add", "add2");
        constexpr auto similarity2 = simhash_similarity("add", "add9");
        static_assert(similarity > similarity2);
    }
}
}