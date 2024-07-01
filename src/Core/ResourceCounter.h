#ifndef RESOURCE_COUNTER_H_
#define RESOURCE_COUNTER_H_

template<typename Resource_t>
class ResourceCounter
{
public:
    ResourceCounter(Resource_t max_, Resource_t consumed_ = 0);
    bool canConsume(Resource_t toConsume_ = 1) const;
    bool consume(Resource_t toConsume_ = 1);
    void free();

private:
    Resource_t m_max;
    Resource_t m_consumed;
};

#endif