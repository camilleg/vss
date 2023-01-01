#include <map>
#include <string>
#include <vector>

class Symtab {
	std::map<std::string, float> m; // The float is either hActor or hNote.
public:
	void Reset();
	void Dump() const;
	bool FAdd(const std::string&, float);
	bool FDelete(const std::string&);
	bool FFound(const std::string&) const;
	float HFromSz(const std::string&) const;
	std::vector<float> Handles() const;
};
