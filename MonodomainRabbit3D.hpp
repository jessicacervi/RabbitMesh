#include <cxxtest/TestSuite.h>
#include "MonodomainProblem.hpp"
#include "LuoRudy1991BackwardEuler.hpp"
#include "GenericMeshReader.hpp"
#include "SimpleStimulus.hpp"
#include "DistributedTetrahedralMesh.hpp"
#include "PetscSetupAndFinalize.hpp"

/** TEMPORARY FOR DEBUGGING */
///\todo #2739
#include <unistd.h>
#include <sys/resource.h>
//#include "Debug.hpp"

class RabbitHeartCellFactory : public AbstractCardiacCellFactory<3> // <3> here
{
private:
    boost::shared_ptr<SimpleStimulus> mpStimulus;

public:
    RabbitHeartCellFactory()
        : AbstractCardiacCellFactory<3>(), // <3> here as well!
          mpStimulus(new SimpleStimulus(-80000.0, 2))
    {
    }

    AbstractCardiacCell* CreateCardiacCellForTissueNode(Node<3>* pNode)
    {
        double z = pNode->rGetLocation()[2];

        if (z <= 0.05)
        {
            return new CellLuoRudy1991FromCellMLBackwardEuler(mpSolver, mpStimulus);
        }
        else
        {
            return new CellLuoRudy1991FromCellMLBackwardEuler(mpSolver, mpZeroStimulus);
        }
    }
};

class TestMonodomain3dRabbitHeartTutorial : public CxxTest::TestSuite
{
public:
    double GetMemoryUsage()
    {
       struct rusage rusage;
       getrusage( RUSAGE_SELF, &rusage );

       return (double)(rusage.ru_maxrss)/(1024);// Convert KB to MB
    }

    void TestMonodomain3dRabbitHeart()
    {
        HeartConfig::Instance()->SetMeshFileName("apps/texttest/weekly/Propagation3d/OxfordRabbitHeart_482um",
                                                 cp::media_type::Axisymmetric);

//        HeartConfig::Instance()->SetMeshFileName("OxfordRabbitHeart_ascii",
//                                                         cp::media_type::Axisymmetric);

        HeartConfig::Instance()->SetIntracellularConductivities(Create_c_vector(1.75, 0.19, 0.19));

        HeartConfig::Instance()->SetSimulationDuration(2); //ms
        HeartConfig::Instance()->SetOutputDirectory("Monodomain3dRabbitHeart");
        HeartConfig::Instance()->SetOutputFilenamePrefix("results");
        HeartConfig::Instance()->SetVisualizeWithVtk(true);

        HeartConfig::Instance()->SetOdePdeAndPrintingTimeSteps(0.02, 0.1, 0.2);

        RabbitHeartCellFactory cell_factory;
        MonodomainProblem<3> monodomain_problem( &cell_factory );
        monodomain_problem.SetWriteInfo();
        monodomain_problem.Initialise();
        monodomain_problem.Solve();


    }
};
