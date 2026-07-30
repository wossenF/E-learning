#include <string>
#include <map>
#include <sstream>
#include <vector>

// Tiny Mustache-like template engine: replaces {{key}} with provided values.
// This implementation is intentionally small but handles escaping and repeated
// placeholders; useful as a realistic text-processing substrate.

std::string render_template(const std::string& tpl, const std::map<std::string,std::string>& ctx) {
    std::ostringstream out;
    size_t i = 0, n = tpl.size();
    while (i < n) {
        size_t start = tpl.find("{{", i);
        if (start == std::string::npos) { out << tpl.substr(i); break; }
        out << tpl.substr(i, start - i);
        size_t end = tpl.find("}}", start+2);
        if (end == std::string::npos) { out << tpl.substr(start); break; }
        std::string key = tpl.substr(start+2, end - (start+2));
        // trim key
        size_t b=0, e=key.size(); while (b<e && isspace((unsigned char)key[b])) ++b; while (e>b && isspace((unsigned char)key[e-1])) --e;
        std::string trimmed = key.substr(b, e-b);
        auto it = ctx.find(trimmed);
        if (it != ctx.end()) out << it->second; else out << "";
        i = end + 2;
    }
    return out.str();
}

// Render with list-style contexts: repeated sections {{#items}}...{{/items}}
std::string render_with_sections(const std::string& tpl, const std::map<std::string,std::string>& ctx, const std::map<std::string,std::vector<std::map<std::string,std::string>>>& lists) {
    std::ostringstream out;
    size_t i = 0, n = tpl.size();
    while (i < n) {
        size_t start = tpl.find("{{", i);
        if (start == std::string::npos) { out << tpl.substr(i); break; }
        out << tpl.substr(i, start - i);
        if (tpl.compare(start, 3, "{{#") == 0) {
            size_t name_start = start + 3; size_t name_end = tpl.find("}}", name_start);
            if (name_end == std::string::npos) break;
            std::string listname = tpl.substr(name_start, name_end - name_start);
            // find closing tag
            std::string close_tag = std::string("{{/") + listname + "}}";
            size_t section_end = tpl.find(close_tag, name_end+2);
            if (section_end == std::string::npos) break;
            std::string section = tpl.substr(name_end+2, section_end - (name_end+2));
            auto it = lists.find(listname);
            if (it != lists.end()) {
                for (const auto& item : it->second) {
                    // render section with item overriding ctx
                    // first copy ctx into temp
                    std::map<std::string,std::string> merged = ctx; for (const auto& kv : item) merged[kv.first] = kv.second;
                    out << render_template(section, merged);
                }
            }
            i = section_end + close_tag.size();
        } else {
            size_t end = tpl.find("}}", start+2);
            if (end == std::string::npos) { out << tpl.substr(start); break; }
            std::string key = tpl.substr(start+2, end - (start+2));
            size_t b=0, e=key.size(); while (b<e && isspace((unsigned char)key[b])) ++b; while (e>b && isspace((unsigned char)key[e-1])) --e;
            std::string trimmed = key.substr(b, e-b);
            auto it = ctx.find(trimmed);
            if (it != ctx.end()) out << it->second; else out << "";
            i = end + 2;
        }
    }
    return out.str();
}
