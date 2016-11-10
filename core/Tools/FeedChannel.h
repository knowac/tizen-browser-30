#ifndef FEEDCHANNEL_H
#define FEEDCHANNEL_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <memory>
#include <list>

#include "FeedItem.h"
#include "EflTools.h"

namespace tizen_browser {
namespace tools {

class FeedChannel
{
public:
    std::string getAuthor() const;
    void setAuthor(const std::string& author);
    std::string getDescription() const;
    void setDescription(const std::string& description);
    tools::FeedItemList getItems() const;
    void setItems(const tools::FeedItemList& items);
    void addItem(std::shared_ptr<tools::FeedItem> item);
    boost::posix_time::ptime getLastBuildDate() const;
    void setLastBuildDate(const boost::posix_time::ptime& lastBuildDate);
    std::string getTitle() const;
    void setTitle(const std::string& title);
    std::string getUrl() const;
    void setUrl(const std::string& url);
    std::string getRealUrl() const;
    void setRealUrl(const std::string& url);
    std::shared_ptr<BrowserImage> getFavicon();
    void setFavicon(std::shared_ptr<BrowserImage>);

private:
    std::string m_title;
    std::string m_description;
    std::string m_url;
    std::string m_realUrl;
    boost::posix_time::ptime m_lastBuildDate;
    std::string m_author;
    tools::FeedItemList m_items;
    std::shared_ptr<BrowserImage> m_favicon;
};

typedef std::vector<std::shared_ptr<tizen_browser::tools::FeedChannel>> FeedChannelList;
typedef std::vector<std::shared_ptr<tizen_browser::tools::FeedChannel>>::iterator FeedChannelListIter;
typedef std::vector<std::shared_ptr<tizen_browser::tools::FeedChannel>>::const_iterator FeedChannelListConstIter;

}
}



#endif // FEEDCHANNEL_H
