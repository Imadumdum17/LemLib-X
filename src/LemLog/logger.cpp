#include "LemLog/logger/Helper.hpp"
#include "LemLog/logger/Sink.hpp"
#include "LemLog/logger/sinks/terminal.hpp"

#include <algorithm>
#include <cstdio>
#include <list>
#include <utility>

namespace {
std::list<logger::Sink*>& sinks() {
    static std::list<logger::Sink*> sinkList;
    return sinkList;
}

const char* levelName(logger::Level level) {
    switch (level) {
        case logger::Level::DEBUG: return "DEBUG";
        case logger::Level::INFO: return "INFO";
        case logger::Level::WARN: return "WARN";
        case logger::Level::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}
} // namespace

namespace logger {
Helper::Helper(const std::string& topic)
    : m_topic(topic) {}

Sink::Sink(std::string name)
    : m_name(std::move(name)) {
    sinks().push_back(this);
}

void Sink::addToAllowList(const std::string& topic) { m_allowList.push_back(topic); }

void Sink::removeFromAllowList(const std::string& topic) { m_allowList.remove(topic); }

void Sink::addToBlockedList(const std::string& topic) { m_blockedList.push_back(topic); }

void Sink::removeFromBlockedList(const std::string& topic) { m_blockedList.remove(topic); }

void Sink::setLoggingLevel(Level level) { m_minLevel = level; }

const std::string& Sink::getName() const& { return m_name; }

Sink::~Sink() { sinks().remove(this); }

SinkStatus Sink::send(Level level, const std::string& topic, const std::string& message) {
    if (static_cast<int>(level) < static_cast<int>(m_minLevel)) return SinkStatus::OK;
    if (!m_allowList.empty() && std::find(m_allowList.begin(), m_allowList.end(), topic) == m_allowList.end()) {
        return SinkStatus::OK;
    }
    if (std::find(m_blockedList.begin(), m_blockedList.end(), topic) != m_blockedList.end()) return SinkStatus::OK;
    return write(level, topic, message);
}

void log(Level level, const std::string& topic, const std::string& message) {
    for (Sink* sink : sinks()) {
        if (sink != nullptr) sink->send(level, topic, message);
    }
}

Terminal::Terminal(bool, bool)
    : Sink("terminal") {}

SinkStatus Terminal::write(Level level, const std::string& topic, const std::string& message) {
    std::printf("[%s] %s: %s\n", levelName(level), topic.c_str(), message.c_str());
    return SinkStatus::OK;
}
} // namespace logger
