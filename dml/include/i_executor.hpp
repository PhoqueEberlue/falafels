class IExecutor
{
public:
    virtual int get_core_count() = 0;

    // Later replace the parameter by a struct holding the model (in case of simulation it will hold the number of flops and in case of real execution it will hold the real model)
    virtual void exec_async(double nb_flops) = 0;
    virtual void wait_all_executions() = 0;
};
