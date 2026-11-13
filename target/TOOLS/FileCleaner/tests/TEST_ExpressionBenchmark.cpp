#include <array>
#include <cstdint>
#include <chrono>
#include <fstream>
#include <memory>
#include <random>

#include "gd/gd_benchmark.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_file.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/io/gd_io_archive_stream.h"
#include "gd/io/gd_io_repository_stream.h"
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_code.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_method_02.h"

extern "C"
{
#include "lua/lua.h"
#include "lua/lauxlib.h"
}

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE("[benchmark] gd-lua code execution", "[benchmark]") 
{
std::string stringCode = R"(
x = 1

if x < 2
    x += 1;
else
    x += 2;
end

function add_to_x( delta )
   x = x + delta;
end

add_to_x( 1 )

for i = 1, i < 10, 1
   x = x + 1;
end
)";

   gd::expression::runtime runtime_;
   runtime_.add({ (unsigned)gd::expression::uMethodDefaultSize_g, gd::expression::pmethodDefault_g, "" });
   runtime_.add({ (unsigned)gd::expression::uMethodStringSize_g, gd::expression::pmethodString_g, "str" });
   runtime_.add("x", int64_t(0));
   gd::expression::code code_;
   auto [bOk, stringError] = code_.compile(stringCode, runtime_);

    INFO(stringError);
    REQUIRE(bOk);

     auto print_result_ = [](const char* pbszName, const gd::benchmark::result& result_)
     {
        std::cout << "\nBenchmark: " << pbszName << "\n"
                  << gd::benchmark::to_string_s(result_, "ns")
                  << "Median: " << result_.median() << " ns\n"
                  << "P95: " << result_.p95() << " ns\n";
     };

    constexpr std::size_t uCompileIterations = 5000;
    constexpr std::size_t uCompileWarmupCount = 100;
    std::string stringCompileError;
    bool bCompilationOk = true;
    auto compile_ = [&]()
    {
       gd::expression::code codeCompiled;
       auto [bCompiled, stringErrorCompile] = codeCompiled.compile(stringCode, runtime_);
       if( !bCompiled ) { stringCompileError = stringErrorCompile; }
       bCompilationOk = bCompilationOk && bCompiled;
       return bCompiled;
    };

    const auto resultCompile = gd::benchmark::run_s( compile_, uCompileIterations, uCompileWarmupCount);

    INFO(stringCompileError);
    CHECK(bCompilationOk);
     print_result_("expression code compilation", resultCompile);

    constexpr std::size_t uRunIterations = 5000;
    constexpr std::size_t uWarmupCount = 100;
    auto setup_ = [&runtime_]() { runtime_.set_variable("x", int64_t(0)); };
    bool bExecutionOk = true;
    auto execute_ = [&]()
    {
       gd::expression::value result_;
       auto error_ = code_.execute(runtime_, &result_);
       bExecutionOk = bExecutionOk && error_.first;
       return error_.first;
    };

    // ## Run benchmark
    const auto resultBenchmark = gd::benchmark::run_prepared_s( setup_, execute_, nullptr, uRunIterations, uWarmupCount);

    CHECK(bExecutionOk);
    gd::expression::value x_(runtime_.get_variable("x"));
    CHECK(x_.as_string() == "11");
     print_result_("expression code execution", resultBenchmark);

   //std::string stringCodeDump = code_.dump();
   //std::cout << "Code dump:\n" << stringCodeDump << std::endl;

}

TEST_CASE("[benchmark] lua code execution", "[benchmark]")
{
   const std::string stringCode = R"(
x = 1

if x < 2 then
   x = x + 1
else
   x = x + 2
end

function add_to_x( delta )
   x = x + delta
end

add_to_x( 1 )

for i = 1, 9 do
   x = x + 1
end
)";

   std::unique_ptr<lua_State, decltype(&lua_close)> pState(luaL_newstate(), &lua_close);
   REQUIRE(pState != nullptr);
   lua_State* pLua = pState.get();

   const int iLoadStatus = luaL_loadbuffer(pLua, stringCode.data(), stringCode.size(), "benchmark");
   if( iLoadStatus != LUA_OK )
   {
      FAIL(lua_tostring(pLua, -1));
   }

   auto print_result_ = [](const char* pbszName, const gd::benchmark::result& result_)
   {
      std::cout << "\nBenchmark: " << pbszName << "\n"
                << gd::benchmark::to_string_s(result_, "ns")
                << "Median: " << result_.median() << " ns\n"
                << "P95: " << result_.p95() << " ns\n";
   };

   constexpr std::size_t uCompileIterations = 5000;
   constexpr std::size_t uCompileWarmupCount = 100;
   std::string stringCompileError;
   bool bCompilationOk = true;
   auto compile_ = [&]()
   {
      const int iStatus = luaL_loadbuffer(pLua, stringCode.data(), stringCode.size(), "benchmark");
      const bool bCompiled = iStatus == LUA_OK;
      if( !bCompiled ) { stringCompileError = lua_tostring(pLua, -1); }
      bCompilationOk = bCompilationOk && bCompiled;
      lua_pop(pLua, 1);
      return bCompiled;
   };

   // Loading parses and compiles without executing; the original chunk stays at stack index 1.
   const auto resultCompile = gd::benchmark::run_s(compile_, uCompileIterations, uCompileWarmupCount);

   INFO(stringCompileError);
   REQUIRE(bCompilationOk);
   CHECK(lua_gettop(pLua) == 1);
   print_result_("lua code compilation", resultCompile);

   constexpr std::size_t uRunIterations = 5000;
   constexpr std::size_t uWarmupCount = 100;
   auto setup_ = [&]()
   {
      lua_pushinteger(pLua, 0);
      lua_setglobal(pLua, "x");
      // lua_pcall consumes its chunk, so copy the retained compiled chunk before timing.
      lua_pushvalue(pLua, 1);
   };
   std::string stringExecutionError;
   bool bExecutionOk = true;
   auto execute_ = [&]()
   {
      const int iStatus = lua_pcall(pLua, 0, 0, 0);
      const bool bExecuted = iStatus == LUA_OK;
      if( !bExecuted )
      {
         stringExecutionError = lua_tostring(pLua, -1);
         lua_pop(pLua, 1);
      }
      bExecutionOk = bExecutionOk && bExecuted;
      return bExecuted;
   };

   const auto resultBenchmark = gd::benchmark::run_prepared_s(setup_, execute_, nullptr,
                                                            uRunIterations, uWarmupCount);

   INFO(stringExecutionError);
   REQUIRE(bExecutionOk);
   CHECK(lua_gettop(pLua) == 1);
   lua_getglobal(pLua, "x");
   REQUIRE(lua_isinteger(pLua, -1));
   CHECK(lua_tointeger(pLua, -1) == 12);
   lua_pop(pLua, 1);
   print_result_("lua code execution", resultBenchmark);
}

namespace {

struct script_options
{
   uint32_t m_uSeed = 3423384420u;
   unsigned m_uFunctionCount = 20;
   unsigned m_uStatementCount = 8;      // statements per function
   unsigned m_uOuterIterations = 50;    // main loop iterations
};

// Generates the same program in Lua syntax (bLua = true) or gd syntax (bLua = false).
// Every random decision is made independently of the dialect, so both outputs are equivalent.
std::string generate_script_s(const script_options& options_, bool bLua)
{
   std::mt19937 generatorRandom(options_.m_uSeed);
   auto random_ = [&generatorRandom](unsigned uLow, unsigned uHigh) -> unsigned
   {
      return uLow + static_cast<unsigned>(generatorRandom() % (uHigh - uLow + 1));
   };

   auto simple_ = [&]() -> std::string
   {
      const unsigned uConstant = random_(1, 9);
      const std::string stringConstant = std::to_string(uConstant);
      switch( random_(0, 3) )
      {
      case 0:  return "x = x + delta * " + stringConstant + ";";
      case 1:  return "y = y + " + stringConstant + ";";
      case 2:  return "z = z + x - y;";
      default: return "x = x - " + stringConstant + ";";
      }
   };

   auto if_ = [bLua](const std::string& stringCondition)
   {
      return std::string("if ") + stringCondition + (bLua ? " then" : "");
   };

   auto for_ = [bLua](const std::string& stringVariable, unsigned uCount)
   {
      std::string stringFor = "for " + stringVariable + " = 1, ";
      if( bLua ) { stringFor += std::to_string(uCount) + " do"; }
      else       { stringFor += stringVariable + " < " + std::to_string(uCount + 1) + ", 1"; }
      return stringFor;
   };

   std::string stringScript;

   // ## Functions
   for( unsigned uFunction = 0; uFunction < options_.m_uFunctionCount; uFunction++ )
   {
      stringScript += "function step_" + std::to_string(uFunction) + "( delta )\n";
      for( unsigned uStatement = 0; uStatement < options_.m_uStatementCount; uStatement++ )
      {
         if( random_(0, 5) >= 4 )
         {
            const unsigned uThreshold = random_(1, options_.m_uOuterIterations);
            const std::string stringThen = simple_();
            const std::string stringElse = simple_();
            stringScript += "   " + if_("delta < " + std::to_string(uThreshold)) + "\n";
            stringScript += "      " + stringThen + "\n";
            stringScript += "   else\n";
            stringScript += "      " + stringElse + "\n";
            stringScript += "   end\n";
         }
         else
         {
            stringScript += "   " + simple_() + "\n";
         }
      }
      stringScript += "end\n\n";
   }

   // ## Main section
   std::vector<unsigned> vectorOrder(options_.m_uFunctionCount);
   std::iota(vectorOrder.begin(), vectorOrder.end(), 0u);
   for( unsigned u = options_.m_uFunctionCount; u > 1; u-- )   // Fisher-Yates
   {
      const unsigned uSwap = random_(0, u - 1);
      std::swap(vectorOrder[u - 1], vectorOrder[uSwap]);
   }

   stringScript += "x = 0;\ny = 0;\nz = 0;\n\n";
   stringScript += for_("i", options_.m_uOuterIterations) + "\n";
   for( unsigned uFunction : vectorOrder )
   {
      stringScript += "   step_" + std::to_string(uFunction) + "( i );\n";
   }
   stringScript += "   " + for_("j", 10) + "\n";
   stringScript += "      z = z + j;\n";
   stringScript += "   end\n";
   stringScript += "   " + if_("x < 1000000") + "\n";
   stringScript += "      x = x + 1;\n";
   stringScript += "   else\n";
   stringScript += "      y = y + 1;\n";
   stringScript += "   end\n";
   stringScript += "end\n";

   return stringScript;
}

void print_result_s(const char* pbszName, const gd::benchmark::result& result_)
{
   std::cout << "\nBenchmark: " << pbszName << "\n"
             << gd::benchmark::to_string_s(result_, "ns")
             << "Median: " << result_.median() << " ns\n"
             << "P95: " << result_.p95() << " ns\n";
}

void register_runtime_s(gd::expression::runtime& runtime_)
{
   runtime_.add({ (unsigned)gd::expression::uMethodDefaultSize_g, gd::expression::pmethodDefault_g, "" });
   runtime_.add({ (unsigned)gd::expression::uMethodStringSize_g, gd::expression::pmethodString_g, "str" });
   runtime_.add("x", int64_t(0));
   runtime_.add("y", int64_t(0));
   runtime_.add("z", int64_t(0));
}

} // namespace


TEST_CASE("[benchmark] large script gives same result in gd-lua and lua", "[benchmark]")
{
   const script_options options_;

   // ## Lua
   const std::string stringLua = generate_script_s(options_, true);
   std::unique_ptr<lua_State, decltype(&lua_close)> pState(luaL_newstate(), &lua_close);
   REQUIRE(pState != nullptr);
   lua_State* pLua = pState.get();
   REQUIRE(luaL_loadbuffer(pLua, stringLua.data(), stringLua.size(), "check") == LUA_OK);
   REQUIRE(lua_pcall(pLua, 0, 0, 0) == LUA_OK);

   // ## gd
   const std::string stringGd = generate_script_s(options_, false);
   gd::expression::runtime runtime_;
   register_runtime_s(runtime_);
   gd::expression::code code_;
   auto [bOk, stringError] = code_.compile(stringGd, runtime_);
   INFO(stringError);
   REQUIRE(bOk);
   gd::expression::value valueResult;
   auto pairExecute = code_.execute(runtime_, &valueResult);
   REQUIRE(pairExecute.first);

   for( const char* pbszName : { "x", "y", "z" } )
   {
      lua_getglobal(pLua, pbszName);
      const std::string stringExpected = std::to_string(lua_tointeger(pLua, -1));
      lua_pop(pLua, 1);
      gd::expression::value valueActual(runtime_.get_variable(pbszName));
      INFO(pbszName);
      CHECK(valueActual.as_string() == stringExpected);
   }
}

TEST_CASE("[benchmark] gd-lua large code execution", "[benchmark]")
{
   const script_options options_;
   const std::string stringCode = generate_script_s(options_, false);
   std::cout << "\ngd script size: " << stringCode.size() << " bytes\n";

   gd::expression::runtime runtime_;
   register_runtime_s(runtime_);
   gd::expression::code code_;
   auto [bOk, stringError] = code_.compile(stringCode, runtime_);
   INFO(stringError);
   REQUIRE(bOk);

   constexpr std::size_t uCompileIterations = 1000;
   constexpr std::size_t uCompileWarmupCount = 20;
   bool bCompilationOk = true;
   auto compile_ = [&]()
   {
      gd::expression::code codeCompiled;
      auto [bCompiled, stringErrorCompile] = codeCompiled.compile(stringCode, runtime_);
      bCompilationOk = bCompilationOk && bCompiled;
      return bCompiled;
   };
   const auto resultCompile = gd::benchmark::run_s(compile_, uCompileIterations, uCompileWarmupCount);
   CHECK(bCompilationOk);
   print_result_s("expression code compilation (large)", resultCompile);

   constexpr std::size_t uRunIterations = 300;
   constexpr std::size_t uWarmupCount = 20;
   auto setup_ = [&runtime_]()
   {
      runtime_.set_variable("x", int64_t(0));
      runtime_.set_variable("y", int64_t(0));
      runtime_.set_variable("z", int64_t(0));
   };
   bool bExecutionOk = true;
   auto execute_ = [&]()
   {
      gd::expression::value result_;
      auto error_ = code_.execute(runtime_, &result_);
      bExecutionOk = bExecutionOk && error_.first;
      return error_.first;
   };
   const auto resultBenchmark = gd::benchmark::run_prepared_s(setup_, execute_, nullptr, uRunIterations, uWarmupCount);
   CHECK(bExecutionOk);
   print_result_s("expression code execution (large)", resultBenchmark);
}

TEST_CASE("[benchmark] lua large code execution", "[benchmark]")
{
   const script_options options_;
   const std::string stringCode = generate_script_s(options_, true);
   std::cout << "\nlua script size: " << stringCode.size() << " bytes\n";

   std::unique_ptr<lua_State, decltype(&lua_close)> pState(luaL_newstate(), &lua_close);
   REQUIRE(pState != nullptr);
   lua_State* pLua = pState.get();

   REQUIRE(luaL_loadbuffer(pLua, stringCode.data(), stringCode.size(), "benchmark") == LUA_OK);

   constexpr std::size_t uCompileIterations = 1000;
   constexpr std::size_t uCompileWarmupCount = 20;
   std::string stringCompileError;
   bool bCompilationOk = true;
   auto compile_ = [&]()
   {
      const int iStatus = luaL_loadbuffer(pLua, stringCode.data(), stringCode.size(), "benchmark");
      const bool bCompiled = iStatus == LUA_OK;
      if( !bCompiled ) { stringCompileError = lua_tostring(pLua, -1); }
      bCompilationOk = bCompilationOk && bCompiled;
      lua_pop(pLua, 1);
      return bCompiled;
   };
   const auto resultCompile = gd::benchmark::run_s(compile_, uCompileIterations, uCompileWarmupCount);
   INFO(stringCompileError);
   REQUIRE(bCompilationOk);
   CHECK(lua_gettop(pLua) == 1);
   print_result_s("lua code compilation (large)", resultCompile);

   constexpr std::size_t uRunIterations = 300;
   constexpr std::size_t uWarmupCount = 20;
   auto setup_ = [&]()
   {
      for( const char* pbszName : { "x", "y", "z" } )
      {
         lua_pushinteger(pLua, 0);
         lua_setglobal(pLua, pbszName);
      }
      lua_pushvalue(pLua, 1);   // lua_pcall consumes its chunk, so push a copy
   };
   std::string stringExecutionError;
   bool bExecutionOk = true;
   auto execute_ = [&]()
   {
      const int iStatus = lua_pcall(pLua, 0, 0, 0);
      const bool bExecuted = iStatus == LUA_OK;
      if( !bExecuted )
      {
         stringExecutionError = lua_tostring(pLua, -1);
         lua_pop(pLua, 1);
      }
      bExecutionOk = bExecutionOk && bExecuted;
      return bExecuted;
   };
   const auto resultBenchmark = gd::benchmark::run_prepared_s(setup_, execute_, nullptr, uRunIterations, uWarmupCount);
   INFO(stringExecutionError);
   REQUIRE(bExecutionOk);
   CHECK(lua_gettop(pLua) == 1);
   print_result_s("lua code execution (large)", resultBenchmark);
}


#include <array>

TEST_CASE("[benchmark] cold start to executed, gd-lua vs lua", "[benchmark]")
{
   struct scenario
   {
      const char* m_pbszName;
      script_options m_options;
      std::size_t m_uIterations;
      std::size_t m_uWarmupCount;
   };

   script_options optionsTiny;
   optionsTiny.m_uFunctionCount = 1;
   optionsTiny.m_uStatementCount = 2;
   optionsTiny.m_uOuterIterations = 10;

   const script_options optionsLarge;   // same defaults as the large tests

   const std::vector<scenario> vectorScenario = {
      { "tiny",  optionsTiny,  5000, 100 },
      { "large", optionsLarge,  300,  20 },
   };

   for( const auto& scenario_ : vectorScenario )
   {
      const std::string stringGd  = generate_script_s(scenario_.m_options, false);
      const std::string stringLua = generate_script_s(scenario_.m_options, true);

      // Everything is created and destroyed inside the lambda, so the timing covers
      // engine creation, setup, compile, execute and teardown.
      // If parray_ is not null the result is read out (use it only outside the timing).
      auto run_gd_ = [&](std::array<int64_t, 3>* parray_) -> bool
      {
         gd::expression::runtime runtime_;
         register_runtime_s(runtime_);
         gd::expression::code code_;
         auto pairCompile = code_.compile(stringGd, runtime_);
         if( !pairCompile.first ) { return false; }
         gd::expression::value valueResult;
         auto pairExecute = code_.execute(runtime_, &valueResult);
         if( !pairExecute.first ) { return false; }
         if( parray_ != nullptr )
         {
            const char* const ppbszName[] = { "x", "y", "z" };
            for( int i = 0; i < 3; i++ )
            {
               gd::expression::value valueVariable(runtime_.get_variable(ppbszName[i]));
               (*parray_)[i] = std::stoll(valueVariable.as_string());
            }
         }
         return true;
      };

      auto run_lua_ = [&](std::array<int64_t, 3>* parray_) -> bool
      {
         std::unique_ptr<lua_State, decltype(&lua_close)> pState(luaL_newstate(), &lua_close);
         if( pState == nullptr ) { return false; }
         lua_State* pLua = pState.get();
         if( luaL_loadbuffer(pLua, stringLua.data(), stringLua.size(), "cold") != LUA_OK ) { return false; }
         if( lua_pcall(pLua, 0, 0, 0) != LUA_OK ) { return false; }
         if( parray_ != nullptr )
         {
            const char* const ppbszName[] = { "x", "y", "z" };
            for( int i = 0; i < 3; i++ )
            {
               lua_getglobal(pLua, ppbszName[i]);
               (*parray_)[i] = static_cast<int64_t>(lua_tointeger(pLua, -1));
               lua_pop(pLua, 1);
            }
         }
         return true;
      };

      // ## Verify that both engines compute the same thing before timing
      std::array<int64_t, 3> arrayGd{}, arrayLua{};
      INFO(scenario_.m_pbszName);
      REQUIRE(run_gd_(&arrayGd));
      REQUIRE(run_lua_(&arrayLua));
      CHECK(arrayGd == arrayLua);

      // ## Time the full lifecycle
      bool bGdOk = true;
      bool bLuaOk = true;
      auto cold_gd_  = [&]() { const bool b = run_gd_(nullptr);  bGdOk  = bGdOk  && b; return b; };
      auto cold_lua_ = [&]() { const bool b = run_lua_(nullptr); bLuaOk = bLuaOk && b; return b; };

      const auto resultGd  = gd::benchmark::run_s(cold_gd_,  scenario_.m_uIterations, scenario_.m_uWarmupCount);
      const auto resultLua = gd::benchmark::run_s(cold_lua_, scenario_.m_uIterations, scenario_.m_uWarmupCount);
      CHECK(bGdOk);
      CHECK(bLuaOk);

      std::cout << "\n=== cold start to executed: " << scenario_.m_pbszName
                << " (gd " << stringGd.size() << " bytes, lua " << stringLua.size() << " bytes) ===";
      print_result_s("gd-lua cold start + compile + execute", resultGd);
      print_result_s("lua cold start + compile + execute", resultLua);
      std::cout << "Median ratio (gd / lua): "
                << static_cast<double>(resultGd.median()) / static_cast<double>(resultLua.median()) << "\n";
   }
}
