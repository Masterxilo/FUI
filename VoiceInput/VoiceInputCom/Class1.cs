using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using FooBillardVoiceInput;
using System.Threading;

namespace VoiceInputCom
{
    [Guid("1E5927B0-F9A8-4E2D-BAA0-255A4D5DCF2A"),
    ClassInterface(ClassInterfaceType.None),
    //ComSourceInterfaces(typeof(IVoiceInputComEvents)),
    ComVisible(true)]
    public class VoiceInputComObject : IVoiceInputComObject
    {
        public VoiceInputComObject()
        {
        }

        public int give5()
        {
            return 6;
        }

        public void init(bool useLimitedDictionary)
        {
            Program.startRecogThread(useLimitedDictionary);
        }

        public bool wordRecognized()
        {
            return Program.wordRecognised();
        }

        public String getLastCommand() 
        {
            return Program.getLastCommand();
        }

        public String[] getRecognizedCommands()
        {
            return Program.getRecongizedCommads();
        }

        public string getLastSpokenPhrase()
        {
            return Program.getLastSpokenPhrase();
        }

    }


    [Guid("1C883E95-D2B5-4FBC-B4BB-5B2251D21061"),
    ComVisible(true)]
    public interface IVoiceInputComObject
    {
        [DispId(1)]
        int give5();
        
        [DispId(2)]
        void init(bool useLimitedDictionary);

        [DispId(3)]
        bool wordRecognized();

        [DispId(4)]
        String getLastCommand();

        [DispId(5)]
        String[] getRecognizedCommands();

        [DispId(6)]
        String getLastSpokenPhrase();
    }
    /*
    [Guid("869BA8A7-6F3F-455D-9FB2-4D0C239AAEA2"),
    InterfaceType(ComInterfaceType.InterfaceIsIDispatch),
    ComVisible(true)]
    public interface IVoiceInputComEvents
    {
    }*/
}
