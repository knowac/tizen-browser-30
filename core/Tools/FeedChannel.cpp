#include "FeedChannel.h"

namespace tizen_browser{
namespace tools{

std::string FeedChannel::getAuthor() const {
    return m_author;
}

void FeedChannel::setAuthor(const std::string& author) {
    this->m_author = author;
}

std::string FeedChannel::getDescription() const {
    return m_description;
}

void FeedChannel::setDescription(const std::string& description) {
    this->m_description = description;
}

FeedItemList FeedChannel::getItems() const {
    return m_items;
}

void FeedChannel::setItems(const FeedItemList& items) {
    this->m_items = items;
}

void FeedChannel::addItem(std::shared_ptr<FeedItem> item)
{
    this->m_items.push_back(item);
}

boost::posix_time::ptime FeedChannel::getLastBuildDate() const {
    return m_lastBuildDate;
}

void FeedChannel::setLastBuildDate(const boost::posix_time::ptime& lastBuildDate) {
    this->m_lastBuildDate = lastBuildDate;
}

std::string FeedChannel::getTitle() const {
    return m_title;
}

void FeedChannel::setTitle(const std::string& title) {
    this->m_title = title;
}

std::string FeedChannel::getUrl() const {
    return m_url;
}

void FeedChannel::setUrl(const std::string& url) {
    this->m_url = url;
}

std::string FeedChannel::getRealUrl() const {
    return m_realUrl;
}

void FeedChannel::setRealUrl(const std::string& url) {
    this->m_realUrl = url;
}

std::shared_ptr<BrowserImage> FeedChannel::getFavicon()
{
    return m_favicon;
}

void FeedChannel::setFavicon(std::shared_ptr<BrowserImage> favicon)
{
    this->m_favicon = favicon;
}

}
}
