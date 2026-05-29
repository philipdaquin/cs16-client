#include "precompiled.h"

CHintMessage::CHintMessage(const char *hintString, bool isHint, CUtlVector<const char *> *args, float duration)
{
	m_hintString = CloneString(hintString);
	m_duration = duration;
	m_isHint = isHint;

	if (args)
	{
		for (int i = 0; i < args->Count(); i++)
			m_args.AddToTail(CloneString((*args)[i]));
	}
}

CHintMessage::~CHintMessage()
{
	m_args.PurgeAndDeleteArrays();

	if (m_hintString)
	{
		delete[] m_hintString;
		m_hintString = NULL;
	}
}

void CHintMessage::Send(CBaseEntity *client)
{
	ALERT(at_console, "[HUD-DBG] CHintMessage::Send msg=%s duration=%.2f isHint=%d client=%s\n",
		m_hintString ? m_hintString : "<null>", m_duration, int(m_isHint),
		(client && client->edict()) ? STRING(client->pev->netname) : "<null>");
	UTIL_ShowMessageArgs(m_hintString, client, &m_args, m_isHint);
}

void CHintMessageQueue::Reset()
{
	m_tmMessageEnd = 0;
	m_messages.PurgeAndDeleteElements();
}

void CHintMessageQueue::Update(CBaseEntity *client)
{
	if (gpGlobals->time <= m_tmMessageEnd)
		return;

	if (!m_messages.Count())
		return;

	CHintMessage *msg = m_messages[0];
	m_tmMessageEnd = gpGlobals->time + msg->GetDuration();
	ALERT(at_console, "[HUD-DBG] CHintMessageQueue::Update sending msg after wait=%.2f\n", msg->GetDuration());
	msg->Send(client);
	delete msg;
	m_messages.Remove(0);
}

bool CHintMessageQueue::AddMessage(const char *message, float duration, bool isHint, CUtlVector<const char *> *args)
{
	CHintMessage *msg = new CHintMessage(message, isHint, args, duration);
	m_messages.AddToTail(msg);
	ALERT(at_console, "[HUD-DBG] CHintMessageQueue::AddMessage queued msg=%s buyHint=%d duration=%.2f isHint=%d args=%d\n",
		message ? message : "<null>",
		(message && !Q_stricmp(message, "#Hint_press_buy_to_purchase")) ? 1 : 0,
		duration, int(isHint), args ? args->Count() : 0);

	return true;
}
