#include <chrono>
#include <fstream>


#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_file.h"
#include "gd/gd_variant.h"
#include "gd/gd_variant_arg.h"
#include "gd/gd_variant_view.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

/// Generate path to data folder where files are located for tests
std::string GetDataFolder()
{
   return FOLDER_GetRoot_g("target/TOOLS/FileCleaner/tests/data");
}

TEST_CASE( "[arguments] test arg and args", "[arguments]" ) {
    // Test arg_view construction
    gd::arg_view av1; gd::variant_view vv1(42); gd::arg_view av2("key1", vv1);
    REQUIRE(av1.empty_key() == true); REQUIRE(av1.empty_value() == true);
    REQUIRE(av2.get_key() == "key1"); REQUIRE(av2.get_value().as_int() == 42);
    
    // Test arg_view methods
    av2.set_key("new_key"); av2.set_value(gd::variant_view("value"));
    REQUIRE(av2.get_key() == "new_key"); REQUIRE(av2.get_value().as_string() == "value");
    
    // Test arg_view comparison
    gd::variant_view vv2(42); gd::arg_view av3("key1", vv2);
    REQUIRE(av2 != av3); av3.set(gd::variant_view("value")); REQUIRE(av2 != av3);
    
    // Test arg construction
    gd::arg a1; gd::variant v1(42); gd::arg a2("key1", v1);
    REQUIRE(a1.empty_key() == true); REQUIRE(a1.empty_value() == true);
    REQUIRE(a2.get_key() == "key1"); REQUIRE(a2.get_value().as_int() == 42);
    
    // Test arg methods
    a2.set_key("new_key"); a2.set_value(gd::variant("value"));
    REQUIRE(a2.get_key() == "new_key"); REQUIRE(a2.get_value().as_string() == "value");
    
    // Test arg comparison
    gd::variant v2(42); gd::arg a3("key1", v2);
    REQUIRE(a2 != a3); a3.set(gd::variant("value")); REQUIRE(a2 != a3);
    
    // Test arg conversion to arg_view
    gd::arg_view av4 = a2; REQUIRE(av4.get_key() == "new_key"); REQUIRE(av4.get_value().as_string() == "value");
    
    // Test args_view construction
    gd::args_view avlist1; gd::args_view avlist2({av2, av3});
    REQUIRE(avlist1.empty() == true); REQUIRE(avlist2.size() == 2);
    
    // Test args_view iteration
    int iCount = 0; for(const auto& av : avlist2) { iCount++; }
    REQUIRE(iCount == 2);
    
    // Test args_view operations
    avlist1.push_back(av2); avlist1.push_back(av3);
    REQUIRE(avlist1.size() == 2); REQUIRE(avlist1.front().get_key() == "new_key");
    REQUIRE(avlist1.back().get_key() == "key1");
    
    // Test args_view find
    auto itFound = avlist1.find("new_key");
    REQUIRE(itFound != avlist1.end()); REQUIRE(itFound->get_key() == "new_key");
    auto itNotFound = avlist1.find("missing");
    REQUIRE(itNotFound == avlist1.end());
    
    // Test args_view contains
    REQUIRE(avlist1.contains("new_key") == true);
    REQUIRE(avlist1.contains("missing") == false);
    
    // Test args_view count
    gd::variant_view vvSame("same"); gd::arg_view avSame1("same", vvSame);
    gd::arg_view avSame2("same", vvSame); avlist1.push_back(avSame1); avlist1.push_back(avSame2);
    REQUIRE(avlist1.count("same") == 2);
    
    // Test args construction
    gd::args alist1; gd::args alist2({a2, a3});
    REQUIRE(alist1.empty() == true); REQUIRE(alist2.size() == 2);
    
    // Test args iteration
    iCount = 0; for(const auto& a : alist2) { iCount++; }
    REQUIRE(iCount == 2);
    
    // Test args operations
    alist1.push_back(a2); alist1.push_back(a3);
    REQUIRE(alist1.size() == 2); REQUIRE(alist1.front().get_key() == "new_key");
    REQUIRE(alist1.back().get_key() == "key1");
    
    // Test args find
    auto itAFount = alist1.find("new_key");
    REQUIRE(itAFount != alist1.end()); REQUIRE(itAFount->get_key() == "new_key");
    auto itANotFound = alist1.find("missing");
    REQUIRE(itANotFound == alist1.end());
    
    // Test args contains
    REQUIRE(alist1.contains("new_key") == true);
    REQUIRE(alist1.contains("missing") == false);
    
    // Test args count
    gd::variant vSame("same"); gd::arg aSame1("same", vSame);
    gd::arg aSame2("same", vSame); alist1.push_back(aSame1); alist1.push_back(aSame2);
    REQUIRE(alist1.count("same") == 2);
    
    // Test args remove
    REQUIRE(alist1.contains("same") == true);
    alist1.remove("same");
    REQUIRE(alist1.contains("same") == false);
    
    // Test args conversion to args_view
    gd::args_view avlist3 = alist1;
    REQUIRE(avlist3.size() == alist1.size());
    for(size_t i = 0; i < alist1.size(); i++) {
        REQUIRE(avlist3[i].get_key() == alist1[i].get_key());
    }
    
    // Test factory functions
    gd::arg a4 = gd::make_arg("test_key", gd::variant(100));
    REQUIRE(a4.get_key() == "test_key"); REQUIRE(a4.get_value().as_int() == 100);
    
    gd::arg_view av5 = gd::make_arg_view("test_key", gd::variant_view(100));
    REQUIRE(av5.get_key() == "test_key"); REQUIRE(av5.get_value().as_int() == 100);
    
    // Test args_from_pairs
    gd::args alist3 = gd::make_args_from_pairs({
        {"key1", gd::variant(1)},
        {"key2", gd::variant(2)}
    });
    REQUIRE(alist3.size() == 2);
    REQUIRE(alist3.find("key1")->get_value().as_int() == 1);
    REQUIRE(alist3.find("key2")->get_value().as_int() == 2);
    
    // Test args_view_from_pairs
    gd::args_view avlist4 = gd::make_args_view_from_pairs({
        {"key1", gd::variant_view(1)},
        {"key2", gd::variant_view(2)}
    });
    REQUIRE(avlist4.size() == 2);
    REQUIRE(avlist4.find("key1")->get_value().as_int() == 1);
    REQUIRE(avlist4.find("key2")->get_value().as_int() == 2);
    
    // Test utility functions
    gd::variant vFound = gd::find_value(alist3, "key1");
    REQUIRE(vFound.as_int() == 1);
    
    gd::variant_view vvFound = gd::find_value(avlist4, "key2");
    REQUIRE(vvFound.as_int() == 2);
    
    // Test get_value_or
    gd::variant vDefault("default");
    gd::variant vOr = gd::get_value_or(alist3, "missing", vDefault);
    REQUIRE(vOr.as_string() == "default");
    
    // Test has_key
    REQUIRE(gd::has_key(alist3, "key1") == true);
    REQUIRE(gd::has_key(alist3, "missing") == false);
    
    // Test get_keys
    auto vectorKeys = gd::get_keys(alist3);
    REQUIRE(vectorKeys.size() == 2);
    REQUIRE(std::find(vectorKeys.begin(), vectorKeys.end(), "key1") != vectorKeys.end());
    REQUIRE(std::find(vectorKeys.begin(), vectorKeys.end(), "key2") != vectorKeys.end());
    
    // Test get_values
    auto vectorValues = gd::get_values(alist3);
    REQUIRE(vectorValues.size() == 2);
    REQUIRE(vectorValues[0].as_int() == 1); REQUIRE(vectorValues[1].as_int() == 2);
    
    // Test filter_args
    gd::args alistFiltered = gd::filter_args(alist3, [](const gd::arg& a) {
        return a.get_key() == "key1";
    });
    REQUIRE(alistFiltered.size() == 1);
    REQUIRE(alistFiltered[0].get_key() == "key1");
    
    // Test transform_args
    gd::args alistTransformed = gd::transform_args(alist3, [](const gd::arg& a) {
        return gd::arg(a.get_key() + "_transformed", gd::variant(a.get_value().as_int() * 10));
    });
    REQUIRE(alistTransformed.size() == 2);
    REQUIRE(alistTransformed[0].get_key() == "key1_transformed");
    REQUIRE(alistTransformed[0].get_value().as_int() == 10);
    
    // Test container operations
    gd::args alist4; alist4.assign({{"assign_key1", gd::variant(10)}, {"assign_key2", gd::variant(20)}});
    REQUIRE(alist4.size() == 2);
    
    alist4.assign(3, gd::arg("default", gd::variant(0)));
    REQUIRE(alist4.size() == 3);
    REQUIRE(alist4[0].get_key() == "default");
    
    // Test emplace operations
    auto itEmplace = alist4.emplace(alist4.begin(), "emplace_key", gd::variant(100));
    REQUIRE(itEmplace->get_key() == "emplace_key");
    REQUIRE(itEmplace->get_value().as_int() == 100);
    REQUIRE(alist4.size() == 4);
    
    gd::arg& refEmplace = alist4.emplace_back("emplace_back_key", gd::variant(200));
    REQUIRE(refEmplace.get_key() == "emplace_back_key");
    REQUIRE(refEmplace.get_value().as_int() == 200);
    REQUIRE(alist4.back().get_key() == "emplace_back_key");
    
    // Test erase operations
    auto itErase = alist4.erase(alist4.begin());
    REQUIRE(itErase->get_key() != "emplace_key");
    REQUIRE(alist4.size() == 4);
    
    itErase = alist4.erase(alist4.begin(), alist4.begin() + 2);
    REQUIRE(itErase->get_key() != "default");
    REQUIRE(alist4.size() == 2);
    
    // Test data access
    gd::arg* pData = alist4.data();
    REQUIRE(pData != nullptr);
    REQUIRE(pData->get_key() == "default");
    
    // Test reverse iteration
    size_t uIndex = alist3.size();
    for(auto it = alist3.rbegin(); it != alist3.rend(); ++it) {
        uIndex--;
        REQUIRE(it->get_key() == alist3[uIndex].get_key());
    }
    
    // Test find_if
    auto itFoundIf = alist3.find_if([](const gd::arg& a) {
        return a.get_key() == "key1";
    });
    REQUIRE(itFoundIf->get_key() == "key1");
    
    // Test find_if_reverse
    auto itFoundReverse = alist3.find_if_reverse([](const gd::arg& a) {
        return a.get_key() == "key2";
    });
    REQUIRE(itFoundReverse->get_key() == "key2");
    
    // Test any_of, all_of, none_of
    REQUIRE(alist3.any_of([](const gd::arg& a) { return a.get_key() == "key1"; }) == true);
    REQUIRE(alist3.all_of([](const gd::arg& a) { return a.get_value().is_int(); }) == true);
    REQUIRE(alist3.none_of([](const gd::arg& a) { return a.get_key() == "missing"; }) == true);
    
    // Test args_view iteration methods
    auto itFoundIfView = avlist4.find_if([](const gd::arg_view& av) {
        return av.get_key() == "key1";
    });
    REQUIRE(itFoundIfView->get_key() == "key1");
    
    auto itFoundReverseView = avlist4.find_if_reverse([](const gd::arg_view& av) {
        return av.get_key() == "key2";
    });
    REQUIRE(itFoundReverseView->get_key() == "key2");
    
    REQUIRE(avlist4.any_of([](const gd::arg_view& av) { return av.get_key() == "key1"; }) == true);
    REQUIRE(avlist4.all_of([](const gd::arg_view& av) { return av.get_value().is_int(); }) == true);
    REQUIRE(avlist4.none_of([](const gd::arg_view& av) { return av.get_key() == "missing"; }) == true);
    
    // Test comparison operators
    gd::args alist5 = gd::make_args_from_pairs({{"key1", gd::variant(1)}, {"key2", gd::variant(2)}});
    gd::args alist6 = gd::make_args_from_pairs({{"key1", gd::variant(1)}, {"key2", gd::variant(2)}});
    gd::args alist7 = gd::make_args_from_pairs({{"key1", gd::variant(1)}, {"key2", gd::variant(3)}});
    REQUIRE(alist5 == alist6); REQUIRE(alist5 != alist7);
    
    gd::args_view avlist5 = gd::make_args_view_from_pairs({{"key1", gd::variant_view(1)}, {"key2", gd::variant_view(2)}});
    gd::args_view avlist6 = gd::make_args_view_from_pairs({{"key1", gd::variant_view(1)}, {"key2", gd::variant_view(2)}});
    gd::args_view avlist7 = gd::make_args_view_from_pairs({{"key1", gd::variant_view(1)}, {"key2", gd::variant_view(3)}});
    REQUIRE(avlist5 == avlist6); REQUIRE(avlist5 != avlist7);
    
    // Test edge cases
    gd::args emptyArgs; gd::args_view emptyArgsView;
    REQUIRE(emptyArgs.empty() == true); REQUIRE(emptyArgs.size() == 0);
    REQUIRE(emptyArgsView.empty() == true); REQUIRE(emptyArgsView.size() == 0);
    
    // Test with empty arg/arg_view
    gd::arg emptyArg; gd::arg_view emptyArgView;
    REQUIRE(emptyArg.empty() == true); REQUIRE(emptyArg.empty_key() == true); REQUIRE(emptyArg.empty_value() == true);
    REQUIRE(emptyArgView.empty() == true); REQUIRE(emptyArgView.empty_key() == true); REQUIRE(emptyArgView.empty_value() == true);
    
    // Test clearing
    alist5.clear(); alist6.clear();
    REQUIRE(alist5.empty() == true); REQUIRE(alist6.empty() == true);
    
    avlist5.clear(); avlist6.clear();
    REQUIRE(avlist5.empty() == true); REQUIRE(avlist6.empty() == true);
    
    // Test conversion between args and args_view
    gd::args alist8 = gd::make_args_from_pairs({{"conv_key1", gd::variant(1)}, {"conv_key2", gd::variant(2)}});
    gd::args_view avlist8 = alist8;
    REQUIRE(avlist8.size() == alist8.size());
    for(size_t i = 0; i < alist8.size(); i++) {
        REQUIRE(avlist8[i].get_key() == alist8[i].get_key());
        REQUIRE(avlist8[i].get_value().as_int() == alist8[i].get_value().as_int());
    }
    
    // Test to_args function
    gd::args alist9 = gd::to_args(avlist8);
    REQUIRE(alist9.size() == avlist8.size());
    for(size_t i = 0; i < avlist8.size(); i++) {
        REQUIRE(alist9[i].get_key() == avlist8[i].get_key());
        REQUIRE(alist9[i].get_value().as_int() == avlist8[i].get_value().as_int());
    }
    
    // Test primitive value factory functions
    gd::arg_view avBool = gd::make_arg_view("bool_key", true);
    REQUIRE(avBool.get_value().as_bool() == true);
    
    gd::arg_view avInt = gd::make_arg_view("int_key", 42);
    REQUIRE(avInt.get_value().as_int() == 42);
    
    gd::arg_view avUint = gd::make_arg_view("uint_key", 42u);
    REQUIRE(avUint.get_value().as_uint() == 42u);
    
    gd::arg_view avInt64 = gd::make_arg_view("int64_key", INT64_C(42));
    REQUIRE(avInt64.get_value().as_int64() == INT64_C(42));
    
    gd::arg_view avUint64 = gd::make_arg_view("uint64_key", UINT64_C(42));
    REQUIRE(avUint64.get_value().as_uint64() == UINT64_C(42));
    
    gd::arg_view avDouble = gd::make_arg_view("double_key", 3.14);
    REQUIRE(avDouble.get_value().as_double() == 3.14);
    
    gd::arg_view avString = gd::make_arg_view("string_key", "test_string");
    REQUIRE(avString.get_value().as_string() == "test_string");
}
