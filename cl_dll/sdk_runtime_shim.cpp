#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <string>
#include <vector>

#include "tier0/dbg.h"
#include "tier0/icommandline.h"
#include "tier0/platform.h"
#include "vstdlib/random.h"

namespace
{
static double CurrentTimeSeconds()
{
	using clock = std::chrono::steady_clock;
	static const clock::time_point s_start = clock::now();
	return std::chrono::duration<double>(clock::now() - s_start).count();
}

static void SpewV(FILE *stream, const char *prefix, const char *fmt, va_list args)
{
	if (prefix && *prefix)
		fputs(prefix, stream);
	vfprintf(stream, fmt, args);
	fflush(stream);
}

class CSimpleCommandLine : public ICommandLine
{
public:
	CSimpleCommandLine()
	{
		CreateCmdLine("");
	}

	virtual void CreateCmdLine(const char *commandline)
	{
		m_CommandLine = commandline ? commandline : "";
		Tokenize();
	}

	virtual void CreateCmdLine(int argc, char **argv)
	{
		m_CommandLine.clear();
		for (int i = 0; i < argc; ++i)
		{
			if (i > 0)
				m_CommandLine.push_back(' ');
			if (argv && argv[i])
				m_CommandLine += argv[i];
		}
		Tokenize();
	}

	virtual const char *GetCmdLine(void) const
	{
		return m_CommandLine.c_str();
	}

	virtual const char *CheckParm(const char *psz, const char **ppszValue = 0) const
	{
		const int index = FindParm(psz);
		if (!index)
			return NULL;

		if (ppszValue)
		{
			const int valueIndex = index + 1;
			if (valueIndex < ParmCount())
				*ppszValue = m_Args[valueIndex].c_str();
			else
				*ppszValue = NULL;
		}

		return m_Args[index].c_str();
	}

	virtual void RemoveParm(const char *parm)
	{
		const int index = FindParm(parm);
		if (!index)
			return;

		m_Args.erase(m_Args.begin() + index);
		RebuildCommandLine();
	}

	virtual void AppendParm(const char *pszParm, const char *pszValues)
	{
		if (pszParm && *pszParm)
			m_Args.emplace_back(pszParm);
		if (pszValues && *pszValues)
			m_Args.emplace_back(pszValues);
		RebuildCommandLine();
	}

	virtual const char *ParmValue(const char *psz, const char *pDefaultVal = 0) const
	{
		const int index = FindParm(psz);
		if (!index)
			return pDefaultVal;

		const int valueIndex = index + 1;
		if (valueIndex >= ParmCount())
			return pDefaultVal;

		return m_Args[valueIndex].c_str();
	}

	virtual int ParmValue(const char *psz, int nDefaultVal) const
	{
		const char *value = ParmValue(psz, static_cast<const char *>(NULL));
		return value ? atoi(value) : nDefaultVal;
	}

	virtual float ParmValue(const char *psz, float flDefaultVal) const
	{
		const char *value = ParmValue(psz, static_cast<const char *>(NULL));
		return value ? static_cast<float>(atof(value)) : flDefaultVal;
	}

	virtual int ParmCount() const
	{
		return static_cast<int>(m_Args.size());
	}

	virtual int FindParm(const char *psz) const
	{
		if (!psz || !*psz)
			return 0;

		for (int i = 1; i < ParmCount(); ++i)
		{
			if (m_Args[i] == psz)
				return i;
		}

		return 0;
	}

	virtual const char *GetParm(int nIndex) const
	{
		if (nIndex < 0 || nIndex >= ParmCount())
			return "";
		return m_Args[nIndex].c_str();
	}

private:
	void Tokenize()
	{
		m_Args.clear();
		m_Args.emplace_back("client");

		std::string token;
		bool inQuotes = false;

		for (char ch : m_CommandLine)
		{
			if (ch == '"')
			{
				inQuotes = !inQuotes;
				continue;
			}

			if (!inQuotes && (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'))
			{
				if (!token.empty())
				{
					m_Args.push_back(token);
					token.clear();
				}
				continue;
			}

			token.push_back(ch);
		}

		if (!token.empty())
			m_Args.push_back(token);
	}

	void RebuildCommandLine()
	{
		m_CommandLine.clear();
		for (size_t i = 1; i < m_Args.size(); ++i)
		{
			if (i > 1)
				m_CommandLine.push_back(' ');
			m_CommandLine += m_Args[i];
		}
	}

	std::string m_CommandLine;
	std::vector<std::string> m_Args;
};

static CSimpleCommandLine g_CommandLine;
static std::mt19937 g_Rng(0x1337u);
static SpewOutputFunc_t g_SpewOutput = NULL;
static SpewType_t g_LastSpewType = SPEW_MESSAGE;
static const char *g_LastSpewFile = "";
static int g_LastSpewLine = 0;
} // namespace

void _AssertValidReadPtr(void *ptr, int count)
{
	(void)count;
	if (!ptr)
		abort();
}

void _AssertValidWritePtr(void *ptr, int count)
{
	(void)count;
	if (!ptr)
		abort();
}

void _AssertValidReadWritePtr(void *ptr, int count)
{
	(void)count;
	if (!ptr)
		abort();
}

void AssertValidStringPtr(const char *ptr, int maxchar)
{
	(void)maxchar;
	if (!ptr)
		abort();
}

void SpewOutputFunc(SpewOutputFunc_t func)
{
	g_SpewOutput = func;
}

SpewOutputFunc_t GetSpewOutputFunc(void)
{
	return g_SpewOutput;
}

void SpewActivate(const tchar *pGroupName, int level)
{
	(void)pGroupName;
	(void)level;
}

bool IsSpewActive(const tchar *pGroupName, int level)
{
	(void)pGroupName;
	(void)level;
	return true;
}

void _SpewInfo(SpewType_t type, tchar const *pFile, int line)
{
	g_LastSpewType = type;
	g_LastSpewFile = pFile ? pFile : "";
	g_LastSpewLine = line;
}

SpewRetval_t _SpewMessage(tchar const *pMsg, ...)
{
	va_list args;
	va_start(args, pMsg);
	SpewV(stderr, "", pMsg, args);
	va_end(args);

	if (g_SpewOutput)
		return g_SpewOutput(g_LastSpewType, pMsg);

	return (g_LastSpewType == SPEW_ERROR) ? SPEW_ABORT : SPEW_CONTINUE;
}

SpewRetval_t _DSpewMessage(tchar const *pGroupName, int level, tchar const *pMsg, ...)
{
	(void)pGroupName;
	(void)level;

	va_list args;
	va_start(args, pMsg);
	SpewV(stderr, "", pMsg, args);
	va_end(args);

	if (g_SpewOutput)
		return g_SpewOutput(g_LastSpewType, pMsg);

	return SPEW_CONTINUE;
}

void _ExitOnFatalAssert(tchar const *pFile, int line)
{
	fprintf(stderr, "Fatal assert at %s:%d\n", pFile ? pFile : "<unknown>", line);
	abort();
}

bool ShouldUseNewAssertDialog()
{
	return false;
}

bool DoNewAssertDialog(const tchar *pFile, int line, const tchar *pExpression)
{
	(void)pFile;
	(void)line;
	(void)pExpression;
	return false;
}

void Msg(tchar const *pMsg, ...)
{
	va_list args;
	va_start(args, pMsg);
	SpewV(stdout, "", pMsg, args);
	va_end(args);
}

void DMsg(tchar const *pGroupName, int level, tchar const *pMsg, ...)
{
	(void)pGroupName;
	(void)level;
	va_list args;
	va_start(args, pMsg);
	SpewV(stdout, "", pMsg, args);
	va_end(args);
}

void Warning(tchar const *pMsg, ...)
{
	va_list args;
	va_start(args, pMsg);
	SpewV(stderr, "Warning: ", pMsg, args);
	va_end(args);
}

void DWarning(tchar const *pGroupName, int level, tchar const *pMsg, ...)
{
	(void)pGroupName;
	(void)level;
	va_list args;
	va_start(args, pMsg);
	SpewV(stderr, "Warning: ", pMsg, args);
	va_end(args);
}

void Log(tchar const *pMsg, ...)
{
	va_list args;
	va_start(args, pMsg);
	SpewV(stdout, "", pMsg, args);
	va_end(args);
}

void DLog(tchar const *pGroupName, int level, tchar const *pMsg, ...)
{
	(void)pGroupName;
	(void)level;
	va_list args;
	va_start(args, pMsg);
	SpewV(stdout, "", pMsg, args);
	va_end(args);
}

void Error(tchar const *pMsg, ...)
{
	va_list args;
	va_start(args, pMsg);
	SpewV(stderr, "Error: ", pMsg, args);
	va_end(args);
	abort();
}

double Plat_FloatTime()
{
	return CurrentTimeSeconds();
}

unsigned long Plat_MSTime()
{
	return static_cast<unsigned long>(CurrentTimeSeconds() * 1000.0);
}

const tchar *Plat_GetCommandLine()
{
	return g_CommandLine.GetCmdLine();
}

void Plat_SetCommandLine(const char *cmdLine)
{
	g_CommandLine.CreateCmdLine(cmdLine);
}

ICommandLine *CommandLine()
{
	return &g_CommandLine;
}

void RandomSeed(int iSeed)
{
	g_Rng.seed(static_cast<unsigned int>(iSeed));
}

float RandomFloat(float flMinVal, float flMaxVal)
{
	if (flMaxVal < flMinVal)
		std::swap(flMinVal, flMaxVal);

	std::uniform_real_distribution<float> dist(flMinVal, flMaxVal);
	return dist(g_Rng);
}

int RandomInt(int iMinVal, int iMaxVal)
{
	if (iMaxVal < iMinVal)
		std::swap(iMinVal, iMaxVal);

	std::uniform_int_distribution<int> dist(iMinVal, iMaxVal);
	return dist(g_Rng);
}

float RandomGaussianFloat(float flMean, float flStdDev)
{
	std::normal_distribution<float> dist(flMean, flStdDev);
	return dist(g_Rng);
}

void InstallUniformRandomStream(IUniformRandomStream *pStream)
{
	(void)pStream;
}
