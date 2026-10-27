#include "tools/tool_registry.h"
#include <cstdint>
#include <utility>
ToolRegistry::ToolRegistry():buckets_(16){}
ToolRegistry::~ToolRegistry()=default;
std::size_t ToolRegistry::hash(std::string_view key) noexcept {
 std::uint64_t h=1469598103934665603ULL;
 for(unsigned char c:key){h^=c;h*=1099511628211ULL;}
 return static_cast<std::size_t>(h);
}
void ToolRegistry::rehash(std::size_t new_count){
 std::vector<std::vector<std::unique_ptr<Tool>>> next(new_count);
 for(auto& chain:buckets_) for(auto& tool:chain)
  next[hash(tool->name())%new_count].push_back(std::move(tool));
 buckets_.swap(next);
}
bool ToolRegistry::add(std::unique_ptr<Tool> tool){
 if(!tool)return false;
 const std::string name=tool->name();
 if(find(name)!=nullptr)return false;
 if((size_+1)*4>buckets_.size()*3)rehash(buckets_.size()*2);
 buckets_[hash(name)%buckets_.size()].push_back(std::move(tool));
 ++size_;return true;
}
Tool* ToolRegistry::find(std::string_view name) const {
 const auto& chain=buckets_[hash(name)%buckets_.size()];
 for(const auto& tool:chain)if(tool->name()==name)return tool.get();
 return nullptr;
}
std::size_t ToolRegistry::size() const noexcept{return size_;}
void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
 for(const auto& chain:buckets_)for(const auto& tool:chain)fn(*tool);
}
std::size_t ToolRegistry::bucket_count() const noexcept{return buckets_.size();}
double ToolRegistry::load_factor() const noexcept{return static_cast<double>(size_)/static_cast<double>(buckets_.size());}
std::size_t ToolRegistry::max_probe_length() const {
 std::size_t longest=0;
 for(const auto& chain:buckets_)if(chain.size()>longest)longest=chain.size();
 return longest;
}
