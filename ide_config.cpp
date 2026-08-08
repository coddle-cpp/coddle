#include "ide_config.hpp"
#include "osal.hpp"
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>

namespace
{
  std::string trim(std::string s)
  {
    while (!s.empty() &&
           (s.front() == ' ' || s.front() == '\t' || s.front() == '\n' || s.front() == '\r'))
      s.erase(s.begin());
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\n' || s.back() == '\r'))
      s.pop_back();
    return s;
  }

  std::string expandShellCommands(std::string flags)
  {
    for (;;)
    {
      auto start = flags.find("$(");
      if (start == std::string::npos)
        break;
      auto end = flags.find(')', start);
      if (end == std::string::npos)
        break;
      auto cmd = flags.substr(start + 2, end - (start + 2));
      auto result = trim(execOut(cmd));
      flags.replace(start, end - start + 1, result);
    }
    return flags;
  }

  std::vector<std::string> tokenize(const std::string &s)
  {
    std::vector<std::string> tokens;
    std::istringstream iss(s);
    std::string tok;
    while (iss >> tok)
      tokens.push_back(tok);
    return tokens;
  }

  void writeIfChanged(const std::string &path, const std::string &content)
  {
    {
      std::ifstream in(path);
      if (in)
      {
        std::ostringstream strm;
        strm << in.rdbuf();
        if (strm.str() == content)
          return;
      }
    }
    std::ofstream out(path);
    out << content;
    std::cout << "Updated " << path << std::endl;
  }

  std::string elispEscape(const std::string &s)
  {
    std::string out;
    out.reserve(s.size());
    for (auto ch : s)
    {
      if (ch == '\\' || ch == '"')
        out.push_back('\\');
      out.push_back(ch);
    }
    return out;
  }

  std::string yamlEscape(const std::string &s)
  {
    return elispEscape(s);
  }

  std::string headerFromPch(std::string pch)
  {
    if (pch.size() > 4 && pch.substr(pch.size() - 4) == ".pch")
      pch = pch.substr(0, pch.size() - 4);
    auto slash = pch.rfind('/');
    if (slash != std::string::npos)
      pch = pch.substr(slash + 1);
    return pch;
  }

  struct ParsedFlags
  {
    std::set<std::string> includes;
    std::set<std::string> definitions;
    std::vector<std::string> args;
  };

  ParsedFlags parseFlags(const std::vector<std::string> &tokens)
  {
    ParsedFlags ret;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
      const auto &t = tokens[i];
      if (t == "-I" && i + 1 < tokens.size())
        ret.includes.insert(tokens[++i]);
      else if (t.rfind("-I", 0) == 0 && t.size() > 2)
        ret.includes.insert(t.substr(2));
      else if (t == "-D" && i + 1 < tokens.size())
        ret.definitions.insert(tokens[++i]);
      else if (t.rfind("-D", 0) == 0 && t.size() > 2)
        ret.definitions.insert(t.substr(2));
      else
        ret.args.push_back(t);
    }
    return ret;
  }

  std::vector<std::string> clangdArgs(const std::vector<std::string> &args)
  {
    std::vector<std::string> ret;
    for (size_t i = 0; i < args.size(); ++i)
    {
      if (args[i] == "-include-pch" && i + 1 < args.size())
      {
        ret.push_back("-include");
        ret.push_back(headerFromPch(args[++i]));
        continue;
      }
      ret.push_back(args[i]);
    }
    return ret;
  }

  std::string renderDirLocals(const ParsedFlags &flags)
  {
    std::ostringstream out;
    out << "((c++-mode . (\n";
    if (!flags.includes.empty())
    {
      out << "              (flycheck-clang-include-path . (\n";
      for (const auto &inc : flags.includes)
        out << "                                              \"" << elispEscape(inc) << "\"\n";
      out << "                                              ))\n";
    }
    if (!flags.definitions.empty())
    {
      out << "              (flycheck-clang-definitions . (\n";
      for (const auto &def : flags.definitions)
        out << "                                             \"" << elispEscape(def) << "\"\n";
      out << "                                             ))\n";
    }
    if (!flags.args.empty())
    {
      out << "              (flycheck-clang-args . (\n";
      for (const auto &arg : flags.args)
        out << "                                      \"" << elispEscape(arg) << "\"\n";
      out << "                                      ))\n";
    }
    out << "              )))\n";
    return out.str();
  }

  std::string renderClangd(const ParsedFlags &flags)
  {
    auto args = clangdArgs(flags.args);
    std::ostringstream out;
    out << "CompileFlags:\n";
    out << "  Add:\n";
    if (!flags.includes.empty())
    {
      out << "    # Includes\n";
      for (const auto &inc : flags.includes)
        out << "    - \"-I" << yamlEscape(inc) << "\"\n";
      out << "\n";
    }
    if (!flags.definitions.empty())
    {
      out << "    # Definitions\n";
      for (const auto &def : flags.definitions)
        out << "    - \"-D" << yamlEscape(def) << "\"\n";
      out << "\n";
    }
    if (!args.empty())
    {
      out << "    # Args\n";
      for (const auto &arg : args)
        out << "    - \"" << yamlEscape(arg) << "\"\n";
    }
    return out.str();
  }
} // namespace

void generateIdeConfigs(const std::string &cflags, const std::string &cxxflags, bool hasNativeLibs)
{
  std::ostringstream all;
  all << " -std=c++26" << cflags << cxxflags;
  if (hasNativeLibs)
    all << " -I.coddle/libs_src";
  auto tokens = tokenize(expandShellCommands(all.str()));
  auto flags = parseFlags(tokens);
  writeIfChanged(".dir-locals.el", renderDirLocals(flags));
  writeIfChanged(".clangd", renderClangd(flags));
}
