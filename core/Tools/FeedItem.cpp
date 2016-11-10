#include "FeedItem.h"

namespace tizen_browser
{
namespace tools
{

std::string FeedItem::getDescription() const {
    return m_description;
}

void FeedItem::setDescription(const std::string& description) {
    this->m_description = description;
}

int FeedItem::getId() const {
    return m_id;
}

void FeedItem::setId(int id) {
    this->m_id = id;
}

boost::posix_time::ptime FeedItem::getPubDate() const {
    return m_pubDate;
}

void FeedItem::setPubDate(const boost::posix_time::ptime& pubDate) {
    this->m_pubDate = pubDate;
}

std::string FeedItem::getTitle() const {
    return m_title;
}

void FeedItem::setTitle(const std::string& title) {
    this->m_title = title;
}

std::string FeedItem::getUrl() const {
    return m_url;
}

void FeedItem::setUrl(const std::string& url) {
    this->m_url = url;
}

std::shared_ptr<BrowserImage> FeedItem::getFavicon()
{
    return m_favicon;
}

void FeedItem::setFavicon(std::shared_ptr<BrowserImage> favicon)
{
    this->m_favicon = favicon;
}

}
}
