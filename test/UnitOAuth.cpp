/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"

//#include "Exceptions.hpp"
#include "Log.hpp"
#include "Unit.hpp"
#include "UnitHTTP.hpp"
#include "helpers.hpp"
#include <Poco/JSON/Object.h>
#include <Poco/LocalDateTime.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/OAuth20Credentials.h>
#include <Poco/Util/LayeredConfiguration.h>

using Poco::DateTimeFormatter;
using Poco::DateTimeFormat;
using Poco::Net::OAuth20Credentials;

class UnitOAuth : public UnitWSD
{
    enum class Phase {
        Load,   // loading the document
        Polling // let the loading progress, and when it succeeds, finish
    } _phase;

public:
    UnitOAuth() :
        _phase(Phase::Load)
    {
    }

    /// Here we act as a WOPI server, so that we have a server that responds to
    /// the wopi requests without additional expensive setup.
    virtual bool handleHttpRequest(const Poco::Net::HTTPRequest& request, std::shared_ptr<StreamSocket>& socket) override
    {
        static const std::string hello("Hello, world");

        Poco::URI uriReq(request.getURI());
        LOG_INF("Fake wopi host request: " << uriReq.toString());

        // CheckFileInfo
        if (uriReq.getPath() == "/wopi/files/0")
        {
            LOG_INF("Fake wopi host request, handling CheckFileInfo.");

            // check that the request contains the Authorization: header
            try {
                OAuth20Credentials creds(request);
                CPPUNIT_ASSERT_EQUAL(creds.getBearerToken(), std::string("s3hn3ct0k3v"));
            }
            catch (const std::exception&)
            {
                // fail as fast as possible
                exit(1);
            }

            Poco::LocalDateTime now;
            Poco::JSON::Object::Ptr fileInfo = new Poco::JSON::Object();
            fileInfo->set("BaseFileName", "hello.txt");
            fileInfo->set("Size", hello.size());
            fileInfo->set("Version", "1.0");
            fileInfo->set("OwnerId", "test");
            fileInfo->set("UserId", "test");
            fileInfo->set("UserFriendlyName", "test");
            fileInfo->set("UserCanWrite", "true");
            fileInfo->set("PostMessageOrigin", "localhost");
            fileInfo->set("LastModifiedTime", DateTimeFormatter::format(now, DateTimeFormat::ISO8601_FORMAT));

            std::ostringstream jsonStream;
            fileInfo->stringify(jsonStream);
            std::string responseString = jsonStream.str();

            const std::string mimeType = "application/json; charset=utf-8";

            std::ostringstream oss;
            oss << "HTTP/1.1 200 OK\r\n"
                << "Last-Modified: " << Poco::DateTimeFormatter::format(Poco::Timestamp(), Poco::DateTimeFormat::HTTP_FORMAT) << "\r\n"
                << "User-Agent: " << WOPI_AGENT_STRING << "\r\n"
                << "Content-Length: " << responseString.size() << "\r\n"
                << "Content-Type: " << mimeType << "\r\n"
                << "\r\n"
                << responseString;

            socket->send(oss.str());
            socket->shutdown();

            return true;
        }
        // GetFile
        else if (uriReq.getPath() == "/wopi/files/0/contents")
        {
            LOG_INF("Fake wopi host request, handling GetFile.");

            // check that the request contains the Authorization: header
            try {
                OAuth20Credentials creds(request);
                CPPUNIT_ASSERT_EQUAL(creds.getBearerToken(), std::string("s3hn3ct0k3v"));
            }
            catch (const std::exception&)
            {
                // fail as fast as possible
                exit(1);
            }

            const std::string mimeType = "text/plain; charset=utf-8";

            std::ostringstream oss;
            oss << "HTTP/1.1 200 OK\r\n"
                << "Last-Modified: " << Poco::DateTimeFormatter::format(Poco::Timestamp(), Poco::DateTimeFormat::HTTP_FORMAT) << "\r\n"
                << "User-Agent: " << WOPI_AGENT_STRING << "\r\n"
                << "Content-Length: " << hello.size() << "\r\n"
                << "Content-Type: " << mimeType << "\r\n"
                << "\r\n"
                << hello;

            socket->send(oss.str());
            socket->shutdown();

            exitTest(TestResult::Ok);

            return true;
        }

        return false;
    }

    void invokeTest() override
    {
        constexpr char testName[] = "UnitOAuth";

        switch (_phase)
        {
            case Phase::Load:
            {
                Poco::URI wopiURL(helpers::getTestServerURI() + "/wopi/files/0?access_token=s3hn3ct0k3v");
                //wopiURL.setPort(_wopiSocket->address().port());
                std::string wopiSrc;
                Poco::URI::encode(wopiURL.toString(), ":/?", wopiSrc);
                Poco::URI loolUri(helpers::getTestServerURI());

                LOG_INF("Connecting to the fake WOPI server: /lool/" << wopiSrc << "/ws");

                std::unique_ptr<UnitWebSocket> ws(new UnitWebSocket("/lool/" + wopiSrc + "/ws"));
                assert(ws.get());

                helpers::sendTextFrame(*ws->getLOOLWebSocket(), "load url=" + wopiSrc, testName);

                _phase = Phase::Polling;
                break;
            }
            case Phase::Polling:
            {
                // let handleHttpRequest() perform the checks...
                break;
            }
        }
    }
};

UnitBase *unit_create_wsd(void)
{
    return new UnitOAuth();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
