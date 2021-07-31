/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOrgueSoundPort.h"

#include "GOrgueSoundRtPort.h"
#include "GOrgueSoundPortaudioPort.h"
#include "GOrgueSoundJackPort.h"
#include "GOrgueSound.h"
#include <wx/intl.h>

static const wxString NAME_DELIM = wxT(": ");
static const size_t NEME_DELIM_LEN = NAME_DELIM.length();

wxString GOrgueSoundPort::NameParser::nextComp()
{
  wxString res("");

  if (hasMore()) {
    size_t newPos = m_Name.find(NAME_DELIM, m_Pos);
    size_t compEnd;
    
    if (newPos != wxString::npos) {
      compEnd = newPos;
      newPos += NEME_DELIM_LEN;
    } else {
      compEnd = m_Name.length();
    }
    res = m_Name.substr(m_Pos, compEnd - m_Pos);
    m_Pos = newPos;
  }
  return res;
}

GOrgueSoundPort::GOrgueSoundPort(GOrgueSound* sound, wxString name) :
	m_Sound(sound),
	m_Index(0),
	m_IsOpen(false),
	m_Name(name),
	m_Channels(0),
	m_SamplesPerBuffer(0),
	m_SampleRate(0),
	m_Latency(0),
	m_ActualLatency(-1)
{
}

GOrgueSoundPort::~GOrgueSoundPort()
{
}

void GOrgueSoundPort::Init(unsigned channels, unsigned sample_rate, unsigned samples_per_buffer, unsigned latency, unsigned index)
{
	m_Index = index;
	m_Channels = channels;
	m_SampleRate = sample_rate;
	m_SamplesPerBuffer = samples_per_buffer;
	m_Latency = latency;
}

void GOrgueSoundPort::SetActualLatency(double latency)
{
	if (latency < m_SamplesPerBuffer / (double)m_SampleRate)
		latency = m_SamplesPerBuffer / (double)m_SampleRate;
	if (latency < 2 * m_SamplesPerBuffer / (double)m_SampleRate)
		latency += m_SamplesPerBuffer / (double)m_SampleRate;
	m_ActualLatency = latency * 1000;
}

bool GOrgueSoundPort::AudioCallback(float* outputBuffer, unsigned int nFrames)
{
	return m_Sound->AudioCallback(m_Index, outputBuffer, nFrames);
}

const wxString& GOrgueSoundPort::GetName()
{
	return m_Name;
}

void append_name(wxString const &nameComp, wxString &resName)
{
  if (! nameComp.IsEmpty()) {
    resName.Append(nameComp);
    resName.Append(NAME_DELIM);
  }
}

wxString GOrgueSoundPort::composeDeviceName(
  wxString const &subsysName,
  wxString const &apiName,
  wxString const &devName
)
{
  wxString resName;
  
  append_name(subsysName, resName);
  append_name(apiName, resName);
  append_name(devName, resName);
  return resName;
}

enum {
  SUBSYS_PA_BIT = 1,
  SUBSYS_RT_BIT = 2,
  SUBSYS_JACK_BIT = 4
};

GOrgueSoundPort* GOrgueSoundPort::create(GOrgueSound* sound, wxString name)
{
  GOrgueSoundPort *port = NULL;
  NameParser parser(name);
  wxString subsysName = parser.nextComp();
  unsigned short subsysMask; // possible subsystems matching with the name
  
  if (subsysName == GOrgueSoundPortaudioPort::getSubsysName())
    subsysMask = SUBSYS_PA_BIT;
  else if (subsysName == GOrgueSoundRtPort::getSubsysName())
    subsysMask = SUBSYS_RT_BIT;
  else if (subsysName == GOrgueSoundJackPort::getSubsysName())
    subsysMask = SUBSYS_JACK_BIT;
  else // old-style name
    subsysMask = SUBSYS_PA_BIT | SUBSYS_RT_BIT | SUBSYS_JACK_BIT;
  
  if (port == NULL && (subsysMask & SUBSYS_PA_BIT))
    port = GOrgueSoundPortaudioPort::create(sound, name);
  if (port == NULL && (subsysMask & SUBSYS_RT_BIT))
    port = GOrgueSoundRtPort::create(sound, name);
  if (port == NULL && (subsysMask & SUBSYS_JACK_BIT))
    port = GOrgueSoundJackPort::create(sound, name);
  return port;
}

std::vector<GOrgueSoundDevInfo> GOrgueSoundPort::getDeviceList()
{
	std::vector<GOrgueSoundDevInfo> result;
	GOrgueSoundPortaudioPort::addDevices(result);
	GOrgueSoundRtPort::addDevices(result);
	GOrgueSoundJackPort::addDevices(result);
	return result;
}

wxString GOrgueSoundPort::getPortState()
{
	if (m_ActualLatency < 0)
		return wxString::Format(_("%s: unknown"), GetName().c_str());
	else
		return wxString::Format(_("%s: %d ms"), GetName().c_str(), m_ActualLatency);
}
