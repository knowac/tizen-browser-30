#ifndef FEEDITEM_H
#define FEEDITEM_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include <EflTools.h>
#include <string>
#include <list>

namespace tizen_browser
{
namespace tools
{

class FeedItem
{
public:
    std::string getDescription() const;
    void setDescription(const std::string& description);
    int getId() const;
    void setId(int id);
    boost::posix_time::ptime getPubDate() const;
    void setPubDate(const boost::posix_time::ptime& pubDate);
    std::string getTitle() const;
    void setTitle(const std::string& title);
    std::string getUrl() const;
    void setUrl(const std::string& url);
    std::shared_ptr<BrowserImage> getFavicon();
    void setFavicon(std::shared_ptr<BrowserImage>);
private:
    std::string m_title;
    std::string m_url;
    int m_id;
    boost::posix_time::ptime m_pubDate;
    std::string m_description;
    std::shared_ptr<BrowserImage> m_favicon;
};

typedef std::list<std::shared_ptr<tizen_browser::tools::FeedItem>> FeedItemList;
typedef std::list<std::shared_ptr<tizen_browser::tools::FeedItem>>::iterator FeedItemListIter;
typedef std::list<std::shared_ptr<tizen_browser::tools::FeedItem>>::const_iterator FeedItemListConstIter;

}
}

#endif // FEEDITEM_H
