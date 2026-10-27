#include "tools/workspace.h"
#include <string>
namespace fs=std::filesystem;
bool Workspace::resolve(std::string_view user_path,fs::path* out,std::string* reason) const {
 if(user_path.empty()){*reason="empty path";return false;}
 if(user_path.find('\0')!=std::string_view::npos){*reason="path contains NUL byte";return false;}
 const fs::path p{std::string(user_path)};
 if(p.is_absolute()||p.has_root_path()){*reason="absolute path not allowed";return false;}
 const fs::path n=p.lexically_normal();
 if(!n.empty() && *n.begin()==fs::path("..")){*reason="path escapes workspace";return false;}
 *out=(n==fs::path("."))?root():root()/n;
 reason->clear();return true;
}
