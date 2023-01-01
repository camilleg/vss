#include <iostream>
#include "symtab.h"

std::vector<float> Symtab::Handles() const {
  std::vector<float> r;
  for (const auto& [_, h]: m)
    r.push_back(h);
  return r;
}

bool Symtab::FAdd(const std::string& sz, float h)
{
	if (FFound(sz)) {
		std::cerr << "FAdd internal error\n";
		return false;
	}
	m[sz] = h;
	return true;
}

bool Symtab::FDelete(const std::string& sz)
{
	return m.erase(sz) != 0;
}

void Symtab::Reset()
{
	m.clear();
}

void Symtab::Dump() const
{
	std::cerr << "Actors:\n";
	for (const auto& [k,v]: m)
		std::cerr << "  " << v << " \"" << k << "\"\n";
}

bool Symtab::FFound(const std::string& sz) const
{
	return m.find(sz) != m.end();
}

float Symtab::HFromSz(const std::string& sz) const
{
	const auto it = m.find(sz);
	return it == m.end() ? -1.0 : it->second;
}
