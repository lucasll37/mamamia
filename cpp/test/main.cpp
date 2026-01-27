// ============================================
// Teste Simples do MIXR
// Testa as bibliotecas básicas sem dependências complexas
// ============================================

#include <iostream>
#include <cstdlib>

// Headers básicos do MIXR
#include "mixr/base/Object.hpp"
#include "mixr/base/String.hpp"
#include "mixr/base/Pair.hpp"
#include "mixr/base/PairStream.hpp"
#include "mixr/base/numeric/Number.hpp"
#include "mixr/base/numeric/Integer.hpp"
#include "mixr/base/units/Angles.hpp"
#include "mixr/base/units/Distances.hpp"
#include "mixr/base/units/Times.hpp"
#include "mixr/base/Timers.hpp"

// ============================================
// Classe de teste simples
// ============================================
namespace test {

class SimpleComponent : public mixr::base::Object
{
   DECLARE_SUBCLASS(SimpleComponent, mixr::base::Object)

public:
   SimpleComponent();
   
   void testBasicTypes();
   void testUnits();
   void testTimers();
   void testCollections();
   
   void runAllTests();

private:
   int test_count_{0};
   int passed_count_{0};
   
   void printTestResult(const char* test_name, bool passed);
};

IMPLEMENT_SUBCLASS(SimpleComponent, "SimpleComponent")
EMPTY_SLOTTABLE(SimpleComponent)
EMPTY_COPYDATA(SimpleComponent)
EMPTY_DELETEDATA(SimpleComponent)

// Construtor
SimpleComponent::SimpleComponent()
{
   STANDARD_CONSTRUCTOR()
   std::cout << "SimpleComponent criado" << std::endl;
}

void SimpleComponent::printTestResult(const char* test_name, bool passed)
{
   test_count_++;
   if (passed) {
      passed_count_++;
      std::cout << "  ✓ " << test_name << std::endl;
   } else {
      std::cout << "  ✗ " << test_name << " FALHOU" << std::endl;
   }
}

// Teste de tipos básicos
void SimpleComponent::testBasicTypes()
{
   std::cout << std::endl;
   std::cout << "========================================" << std::endl;
   std::cout << "Testando Tipos Básicos do MIXR" << std::endl;
   std::cout << "========================================" << std::endl;
   
   // Teste String
   mixr::base::String* str = new mixr::base::String("Hello MIXR!");
   bool test1 = (str != nullptr && str->len() == 11);
   printTestResult("String creation", test1);
   str->unref();
   
   // Teste Integer
   mixr::base::Integer* num = new mixr::base::Integer(42);
   bool test2 = (num != nullptr && num->asInt() == 42);
   printTestResult("Integer creation", test2);
   num->unref();
   
   // Teste Number (double)
   mixr::base::Number* dbl = new mixr::base::Number(3.14159);
   bool test3 = (dbl != nullptr && dbl->asDouble() > 3.14 && dbl->asDouble() < 3.15);
   printTestResult("Number (double) creation", test3);
   dbl->unref();
}

// Teste de unidades
void SimpleComponent::testUnits()
{
   std::cout << std::endl;
   std::cout << "========================================" << std::endl;
   std::cout << "Testando Unidades do MIXR" << std::endl;
   std::cout << "========================================" << std::endl;
   
   // Teste Ângulos
   mixr::base::Degrees* deg = new mixr::base::Degrees(180.0);
   double rad = deg->getValueInRadians();
   bool test1 = (rad > 3.14 && rad < 3.15);  // ~PI
   printTestResult("Degrees to Radians", test1);
   std::cout << "    180° = " << rad << " rad" << std::endl;
   deg->unref();
   
   // Teste Distâncias
   mixr::base::Meters* m = new mixr::base::Meters(1000.0);
   double km = m->getValueInKiloMeters();
   bool test2 = (km == 1.0);
   printTestResult("Meters to Kilometers", test2);
   std::cout << "    1000m = " << km << " km" << std::endl;
   m->unref();
   
   // Teste Tempo
   mixr::base::Seconds* sec = new mixr::base::Seconds(60.0);
   double min = sec->getValueInMinutes();
   bool test3 = (min == 1.0);
   printTestResult("Seconds to Minutes", test3);
   std::cout << "    60s = " << min << " min" << std::endl;
   sec->unref();
}

// Teste de timers
void SimpleComponent::testTimers()
{
   std::cout << std::endl;
   std::cout << "========================================" << std::endl;
   std::cout << "Testando Timers do MIXR" << std::endl;
   std::cout << "========================================" << std::endl;
   
   // Criar timer
   mixr::base::Timer timer;
   
   // Configurar para 1 segundo
   timer.setTime(1.0);
   timer.start();
   
   bool test1 = timer.isRunning();
   printTestResult("Timer start", test1);
   
   // Simular passagem de tempo
   timer.updateTimers(0.5);  // Atualiza 0.5 segundos
   
   bool test2 = !timer.alarm();  // Não deve ter disparado ainda
   printTestResult("Timer running (not alarmed)", test2);
   std::cout << "    Tempo restante: " << timer.getTimeLeft() << "s" << std::endl;
   
   // Atualizar mais 0.6 segundos (total 1.1s)
   timer.updateTimers(0.6);
   
   bool test3 = timer.alarm();  // Deve ter disparado
   printTestResult("Timer alarm", test3);
}

// Teste de coleções
void SimpleComponent::testCollections()
{
   std::cout << std::endl;
   std::cout << "========================================" << std::endl;
   std::cout << "Testando Coleções do MIXR" << std::endl;
   std::cout << "========================================" << std::endl;
   
   // Criar PairStream (lista de pares nome/valor)
   mixr::base::PairStream* list = new mixr::base::PairStream();
   
   // Adicionar items
   list->put(new mixr::base::Pair("idade", new mixr::base::Integer(25)));
   list->put(new mixr::base::Pair("nome", new mixr::base::String("Lucas")));
   list->put(new mixr::base::Pair("altura", new mixr::base::Number(1.75)));
   
   bool test1 = (list->entries() == 3);
   printTestResult("PairStream creation", test1);
   std::cout << "    Entradas: " << list->entries() << std::endl;
   
   // Buscar por nome
   mixr::base::Pair* pair = list->findByName("idade");
   bool test2 = (pair != nullptr);
   printTestResult("PairStream find by name", test2);
   
   if (pair) {
      mixr::base::Integer* idade = dynamic_cast<mixr::base::Integer*>(pair->object());
      if (idade) {
         std::cout << "    idade = " << idade->asInt() << std::endl;
      }
   }
   
   list->unref();
}

// Executar todos os testes
void SimpleComponent::runAllTests()
{
   std::cout << std::endl;
   std::cout << "========================================" << std::endl;
   std::cout << "INICIANDO TESTES DO MIXR" << std::endl;
   std::cout << "========================================" << std::endl;
   
   testBasicTypes();
   testUnits();
   testTimers();
   testCollections();
   
   std::cout << std::endl;
   std::cout << "========================================" << std::endl;
   std::cout << "RESUMO DOS TESTES" << std::endl;
   std::cout << "========================================" << std::endl;
   std::cout << "Total de testes: " << test_count_ << std::endl;
   std::cout << "Testes passou:   " << passed_count_ << std::endl;
   std::cout << "Testes falhou:   " << (test_count_ - passed_count_) << std::endl;
   
   if (passed_count_ == test_count_) {
      std::cout << std::endl;
      std::cout << "✓ TODOS OS TESTES PASSARAM!" << std::endl;
   } else {
      std::cout << std::endl;
      std::cout << "✗ ALGUNS TESTES FALHARAM" << std::endl;
   }
   std::cout << "========================================" << std::endl;
}

} // namespace test

// ============================================
// Main
// ============================================
int main(int argc, char* argv[])
{
   std::cout << "========================================" << std::endl;
   std::cout << "MIXR - Teste de Instalação" << std::endl;
   std::cout << "========================================" << std::endl;
   std::cout << std::endl;
   
   // Verificar versão/informações do MIXR
   std::cout << "Informações do sistema:" << std::endl;
   std::cout << "  Compilador: " << __VERSION__ << std::endl;
   std::cout << "  C++ Standard: " << __cplusplus << std::endl;
   std::cout << std::endl;
   
   // Criar e executar testes
   test::SimpleComponent* testObj = new test::SimpleComponent();
   
   if (testObj != nullptr) {
      testObj->runAllTests();
      testObj->unref();
   } else {
      std::cerr << "ERRO: Não foi possível criar SimpleComponent" << std::endl;
      return 1;
   }
   
   std::cout << std::endl;
   std::cout << "Teste concluído!" << std::endl;
   std::cout << std::endl;
   
   return 0;
}