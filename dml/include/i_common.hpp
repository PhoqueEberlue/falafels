class ICommon
{
public:
    virtual ~ICommon();
    virtual double get_time() = 0;
    virtual void kill_processes() = 0;
};
