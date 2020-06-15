#include <class.h>
#include <file.h>
#include <printer.h>

int main(int argc, char **argv) {
  KODE::Class mainClass("MainClass");
  KODE::Class nestedClass("NestedClass");
  nestedClass.setUseDPointer(true);

  // Create MainClass functions
  KODE::Function mainFunction1("mainFunction1", "void");
  mainClass.addFunction(mainFunction1);

  // Create NestedClass functions
  KODE::Function nestedFunction1("nestedFunction1", "void");
  nestedClass.addFunction(nestedFunction1);

  KODE::MemberVariable nestedMember1("name", "QString");
  nestedClass.addMemberVariable(nestedMember1);

  // Add NestedClass to MainClass
  mainClass.addNestedClass(nestedClass);

  KODE::Printer printer;
  KODE::File file;

  file.setFilename("generatednestedclass");

  file.insertClass(mainClass);

  printer.printHeader(file);
  printer.printImplementation(file);

  return 0;
}
