#include "entity_registry.h"
#include <iostream>
#include <stdexcept>
using namespace inventory_next;
void check(bool ok,const char* message) { if(!ok) throw std::runtime_error(message); }
void yes(Result result) { if(!result) throw std::runtime_error(result.error); }
int main() {
    try {
        EntityRegistry r; yes(r.restore_known({10,20,30})); yes(r.bind(10,5));
        const auto first=*r.handle(10); check(r.resolve(first)==10,"initial resolve");
        yes(r.bind(10,5)); check(r.handle(10)->generation==first.generation,"idempotent bind changed generation");
        check(!r.bind(20,5)&&!r.bind(10,6)&&!r.bind(99,6)&&!r.bind(20,0xffff),"invalid binding accepted");
        check(r.online_count()==1&&r.resolve(first)==10,"failed bind altered registry");
        std::cout<<"PASS unique bindings and invalid IDs\n";
        yes(r.unbind(first)); yes(r.bind(20,5)); const auto second=*r.handle(20);
        check(!r.resolve(first)&&!r.unbind(first)&&r.resolve(second)==20,"reused native ID accepted stale handle");
        yes(r.unbind(second)); yes(r.bind(10,5)); check(!r.resolve(first),"same object rebind revived old handle");
        std::cout<<"PASS native slot reuse and same-item rebind generations\n";
        auto current=*r.handle(10); const auto epoch=r.epoch();
        check(!r.restore_known({10,10})&&!r.restore_known({0}),"invalid restore accepted");
        check(r.epoch()==epoch&&r.resolve(current)==10&&r.known(30),"invalid restore was not atomic");
        yes(r.restore_known({10,20})); yes(r.bind(10,5)); check(!r.resolve(current),"reload revived live binding");
        check(!r.known(30)&&r.known(20)&&!r.handle(20),"offline membership incorrect");
        std::cout<<"PASS transactional identity restore and offline items\n";
        EntityRegistry other; yes(other.restore_known({10})); yes(other.bind(10,5));
        check(!r.resolve(*other.handle(10))&&!other.resolve(*r.handle(10)),"cross-registry handle accepted");
        auto forged=*r.handle(10); forged.live=6; check(!r.resolve(forged),"wrong live slot accepted");
        std::cout<<"PASS registry isolation and handle field validation\n";
        EntityRegistry many; std::vector<Id> ids; for(Id i=1;i<=4096;++i) ids.push_back(i);
        yes(many.restore_known(ids)); check(!many.add(4097),"registry limit bypassed"); yes(many.add(1));
        check(!r.forget(10),"forgot live object"); yes(many.forget(4096)); yes(many.add(4097));
        check(!many.known(4096)&&many.known(4097),"destroyed object retirement");
        for(unsigned i=0;i<1000;++i) { yes(r.bind(20,10)); const auto h=*r.handle(20); yes(r.unbind(h)); check(!r.resolve(h),"lifecycle stale handle"); }
        std::cout<<"PASS bounded identities and 1000 online/offline cycles\n";
        return 0;
    } catch(const std::exception& e) { std::cerr<<"FAIL "<<e.what()<<'\n'; return 1; }
}
