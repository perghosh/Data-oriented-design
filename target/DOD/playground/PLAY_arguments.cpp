#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"

#include "gd/gd_arguments_common.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


void print( const gd::argument::arguments arguments_ )
{
   std::cout << arguments_.print() << std::endl;
}

TEST_CASE( "add from arguments", "[arguments]" ) {
   gd::argument::arguments arguments_( { {"1", 1}, {"1", 1}, {"1", 1} } );

   // how to do a one liner
   print({ { {"2", 2}, { "3", 3 } }, arguments_});
   print({ arguments_, { {"1", 1}, {"1", 1} } });
}



// Run logic on arguments to test new features --------------------------------
TEST_CASE( "[arguments] add variables", "[arguments]" ) {
   gd::argument::arguments arguments_;

   arguments_.append_many(100, 200, 300, 400, 500);

   arguments_.append( "1000", 1000 );

   gd::argument::arguments_value AV_( &arguments_ );
   AV_["test"] = "test";
   AV_["test1"] = "test1";
   AV_["test2"] = "test2";
   arguments_.append("test", "test");
   auto s_ = arguments_.get_variant_view("test").as_string();
   arguments_.append("test1", "test1");
   arguments_.append("test2", "test2");

   auto pfind_ = arguments_.find( "test2" );

   auto stringDump = arguments_.print();

   AV_["test1"] = "xxxxx";

   stringDump = arguments_.print();

   {
      auto pa_ = AV_.get_arguments();
      auto pp_ = AV_.get_position();
      assert( arguments_.buffer_data() == pa_->buffer_data() );
      uint64_t u = pp_ - pa_->buffer_data();
      std::cout << "Position: " << u << std::endl;
   }

   gd::variant_view v_ = AV_;
   std::string stringValue = v_.as_string();

   stringDump = arguments_.print();
   std::cout << stringDump << std::endl;

   AV_["dump"] = stringDump;

   //gd::com::pointer
}

// Run logic on arguments to test new features --------------------------------
TEST_CASE( "[arguments] add shared variables", "[arguments]" ) {
   gd::argument::shared::arguments arguments_;

   arguments_.append_many(100, 200, 300, 400, 500);

   arguments_.append( "1000", 1000 );

   gd::argument::arguments_value AV_( &arguments_ );
   AV_["test"] = "test";
   AV_["test1"] << "test1";
   AV_["test2"] = "test2";
   auto stringDump = arguments_.print();
   std::cout << stringDump << std::endl;
   arguments_.append("test", "test");
   auto s_ = arguments_.get_variant_view("test").as_string();
   arguments_.append("test1", "test1");
   arguments_.append("test2", "test2");

   auto pfind_ = arguments_.find( "test2" );

   stringDump = arguments_.print();

   AV_["test1"] = "xxxxx";
   gd::variant_view variantviewOut;
   AV_ >> variantviewOut;
   auto ss_ = variantviewOut.as_string();
   AV_ >> ss_;

   gd::variant_view variantview_( "Hello World!" );
   std::string stringText = variantview_.as<std::string>();
   std::cout << stringText << std::endl;
   auto stringAlsoText = variantview_.as<decltype(stringText)>();
   std::cout << stringText << std::endl;
   assert( stringText == stringAlsoText );


   stringDump = arguments_.print();

   {
      auto pa_ = AV_.get_arguments();
      auto pp_ = AV_.get_position();
      assert( arguments_.buffer_data() == pa_->buffer_data() );
      uint64_t u = pp_ - pa_->buffer_data();
      std::cout << "Position: " << u << std::endl;
   }

   {
      gd::argument::shared::arguments arguments_;
      gd::argument::arguments_value AV_( &arguments_ );
      AV_ << 1 << 2 << 3 << 4 << 5;
      int i1, i2, i3, i4, i5;
      gd::argument::arguments_value AVRead( &arguments_ );
      AVRead >> i1 >> i2 >> i3 >> i4 >> i5;
   }

   gd::variant_view v_ = AV_;
   std::string stringValue = v_.as_string();

   stringDump = arguments_.print();
   std::cout << stringDump << std::endl;

   //AV_["dump"] = stringDump;

   //gd::com::pointer
}