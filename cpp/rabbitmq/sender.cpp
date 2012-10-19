/**
 * Copyright (c) dushin.net
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of dushin.net nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY dushin.net ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL dushin.net BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdlib>
#include <iostream>
#include <memory>
#include <boost/program_options.hpp>
#include <SimpleAmqpClient/SimpleAmqpClient.h>

using namespace AmqpClient;

#define VERBOSE(X) if (verbose) { std::cout << X << std::endl; }

std::string
random_data(unsigned len)
{
    std::auto_ptr<char> data(new char[len]);
    for (unsigned i = 0;  i < len;  ++i)
    {
        data.get()[i] = ' ' + (::rand() % 100);
    }
    return std::string(data.get(), len);
}


int main(int argc, char** argv)
{
    std::string hostname;
    std::string exchange_name;
    std::string queue_name;
    unsigned num_messages;
    unsigned message_size;
    bool verbose;
    boost::program_options::options_description syntax("Syntax");
    syntax.add_options()
        ("help", "print this help message")
        ("hostname", boost::program_options::value<std::string>(&hostname)->default_value("localhost"), "AMQP Broker host")
        ("exchange_name", boost::program_options::value<std::string>(&exchange_name)->default_value(""), "Exchange name")
        ("queue_name", boost::program_options::value<std::string>(&queue_name)->default_value("test"), "Queue name")
        ("num_messages", boost::program_options::value<unsigned>(&num_messages)->default_value(1), "Number of messages to publish")
        ("message_size", boost::program_options::value<unsigned>(&message_size)->default_value(1024), "Message size")
        ("verbose", boost::program_options::value<bool>(&verbose)->default_value(false), "Verbose output")
    ;
    boost::program_options::variables_map vm;
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, syntax), vm
    );
    boost::program_options::notify(vm);
    if (vm.count("help")) {
        std::cout << syntax << std::endl;
        return 0;
    }
    try
    {
        Channel::ptr_t channel(Channel::Create(hostname));
        if (exchange_name.length() > 0)
        {
            const std::string exchange_type(Channel::EXCHANGE_TYPE_DIRECT);
            const bool passive(false);
            const bool durable(false);
            const bool auto_delete(false);
            channel->DeclareExchange(exchange_name, exchange_type, passive, durable, auto_delete);
        }
        {
            const bool passive(false);
            const bool durable(false);
            const bool exclusive(false);
            const bool auto_delete(false);
            channel->DeclareQueue(queue_name, passive, durable, exclusive, auto_delete);
        }
        std::string routing_key;
        if (exchange_name.length() > 0)
        {
            VERBOSE(
                "BindQueue"
                << "; exchange_name = " << exchange_name
                << "; queue_name = " << queue_name
            );
            channel->BindQueue(queue_name, exchange_name);
        }
        else
        {
            routing_key = queue_name;
        }

        VERBOSE(
            "BasicPublish"
            << "; exchange_name = " << exchange_name
            << "; routing_key = " << routing_key
        );
        const std::string msg(random_data(message_size));
        for (unsigned i = 0;  i < num_messages;  ++i)
        {
            channel->BasicPublish(exchange_name, routing_key, BasicMessage::Create(msg));
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }
}
