#include "../src/xrGame/inventory_grid.h"
#include <cassert>
#include <climits>
#include <iostream>
#include <random>
#include <set>
#include <sstream>

using namespace inventory_grid;
static int checks = 0;
static void expect(bool ok) { ++checks; if (!ok) { std::cerr << "check " << checks << " failed\n"; std::abort(); } }
static std::string snapshot(const Grid& g)
{
    std::ostringstream s;
    s << g.columns() << ',' << g.rows() << ';';
    for (const Placement& p : g.entries())
        s << p.key << ':' << p.x << ':' << p.y << ':' << p.width << ':' << p.height << ':' << p.rotated << ';';
    return s.str();
}
static void validate(const Grid& g)
{
    std::set<Key> keys;
    for (const Placement& p : g.entries())
    {
        expect(keys.insert(p.key).second);
        expect(p.x >= 0 && p.y >= 0);
        expect(p.x + p.columns() <= g.columns() && p.y + p.rows() <= g.rows());
        for (const Placement& q : g.entries())
            if (p.key != q.key)
                expect(p.x + p.columns() <= q.x || q.x + q.columns() <= p.x ||
                    p.y + p.rows() <= q.y || q.y + q.rows() <= p.y);
    }
}
int main()
{
    Grid g;
    expect(g.place({1,0,0,1,1,false}) == Result::outside);
    expect(g.resize(4,3) == Result::ok);
    expect(g.resize(0,3) == Result::invalid_size);
    expect(g.place({1,0,0,2,1,false}) == Result::ok);
    const auto before = snapshot(g);
    expect(g.place({2,1,0,2,1,false}) == Result::overlap);
    expect(snapshot(g) == before);
    expect(g.move(1,-1,0,false) == Result::outside);
    expect(g.move(1,INT_MAX,0,false) == Result::outside);
    expect(g.place({2,0,0,INT_MAX,1,false}) == Result::invalid_size);
    expect(snapshot(g) == before);
    expect(g.move(1,3,0,true) == Result::ok);
    expect(g.find(1)->rotated && g.find(1)->columns() == 1);
    expect(g.move(999,0,0,false) == Result::missing_item);
    expect(g.resize(3,3) == Result::outside);
    expect(g.columns() == 4);
    Grid b;
    expect(b.resize(1,2) == Result::ok);
    expect(g.transfer(1,b,0,0,false) == Result::outside);
    expect(g.find(1) != nullptr && b.find(1) == nullptr);
    expect(g.transfer(1,b,0,0,true) == Result::ok);
    expect(g.find(1) == nullptr && b.find(1) != nullptr);
    expect(b.transfer(1,b,0,0,true) == Result::ok);
    Placement fit{999,99,99,1,1,false};
    expect(g.first_fit(10,3,4,false,true,fit) == Result::ok);
    expect(fit.x == 0 && fit.y == 0 && fit.rotated);
    auto keep = fit;
    expect(b.first_fit(10,1,1,false,false,fit) != Result::ok);
    expect(fit.key == keep.key && fit.x == keep.x && fit.rotated == keep.rotated);
    expect(g.place({1,0,0,1,1,false}) == Result::ok);
    expect(b.transfer(1,g,2,2,true) == Result::duplicate_key);
    expect(b.find(1) != nullptr);

    Grid s;
    expect(s.resize(5,3) == Result::ok);
    expect(s.place({1,0,0,2,1,false}) == Result::ok);
    expect(s.place({2,3,0,1,1,false}) == Result::ok);
    expect(s.swap(1,s,2) == Result::ok);
    expect(s.find(1)->x == 3 && s.find(2)->x == 0);
    expect(s.entries().size() == 2);
    const auto stable = snapshot(s);
    expect(s.swap(1,s,999) == Result::missing_item && snapshot(s) == stable);
    Grid t;
    expect(t.resize(1,1) == Result::ok);
    expect(t.place({3,0,0,1,1,false}) == Result::ok);
    expect(s.swap(1,t,3) == Result::outside && snapshot(s) == stable);
    expect(t.find(3) != nullptr);
    expect(s.swap(2,t,3) == Result::ok);
    expect(s.find(3) && t.find(2) && !s.find(2) && !t.find(3));

    Grid r;
    expect(r.restore(s.columns(),s.rows(),s.entries()) == Result::ok);
    expect(snapshot(r) == snapshot(s));
    const auto saved = snapshot(r);
    expect(r.restore(4,4,{{1,0,0,1,1,false},{1,1,1,1,1,false}}) == Result::duplicate_key);
    expect(snapshot(r) == saved);
    expect(r.restore(4,4,{{1,0,0,2,2,false},{2,1,1,1,1,false}}) == Result::overlap);
    expect(snapshot(r) == saved);
    expect(r.restore(4,4,{{1,4,0,1,1,false}}) == Result::outside);
    expect(snapshot(r) == saved);
    expect(r.restore(4,4,{}) == Result::ok && r.entries().empty());

    // Reproduce the original conceptual bug: a filled rig contributes a single
    // outer footprint; its contents live in a different grid.
    Grid outer, rig;
    expect(outer.resize(2,2) == Result::ok && rig.resize(4,4) == Result::ok);
    expect(outer.place({100,0,0,1,1,false}) == Result::ok);
    for (Key k=1;k<=16;++k)
        expect(rig.place({k,int((k-1)%4),int((k-1)/4),1,1,false}) == Result::ok);
    expect(outer.place({101,1,0,1,1,false}) == Result::ok);
    expect(outer.entries().size()==2 && rig.entries().size()==16);

    // Seeded operations check invariants and no mutation on rejection.
    std::mt19937 random(5636835);
    Grid a,c;
    a.resize(8,8); c.resize(8,8);
    for (int i=0;i<10000;++i)
    {
        const auto old_a=snapshot(a), old_c=snapshot(c);
        Grid& from = (random()%2) ? a : c;
        Grid& to = (&from==&a) ? c : a;
        Key key=random()%30, other_key=random()%30;
        int x=int(random()%12)-2, y=int(random()%12)-2;
        Result result;
        switch(random()%5)
        {
        case 0: result=from.place({key,x,y,int(random()%4)+1,int(random()%4)+1,bool(random()%2)}); break;
        case 1: result=from.move(key,x,y,bool(random()%2)); break;
        case 2: result=from.transfer(key,to,x,y,bool(random()%2)); break;
        case 3: result=from.swap(key,to,other_key); break;
        default: result=from.swap(key,from,other_key); break;
        }
        if(result != Result::ok)
            expect(snapshot(a)==old_a && snapshot(c)==old_c);
        validate(a); validate(c);
    }
    std::cout << checks << " checks passed; 10000 seeded operations\n";
}

