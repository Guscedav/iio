/**
 * @mainpage
 * <h4>The Industrial Input/Output Library: libiio.so</h4>
 * This library offers classes to access industrial periphery hardware typically used in
 * industrial control and automation.
 * <br/>
 * The following UML class diagram shows an overview of the classes of this library and their
 * association.
 * <br/><br/>
 * <div style="text-align:center"><img src="iio.png" width="800"/></div>
 * <div style="text-align:center"><b>Overview of the classes of this library</b></div>
 * <br/>
 * Individual inputs and outputs are represented by an abstract channel class. Specific channel
 * classes are <code>AnalogIn</code>, <code>AnalogOut</code>, <code>DigitalIn</code> or
 * <code>DigitalOut</code>. These specific inputs and outputs offer <code>read()</code> or
 * <code>write()</code> methods to read the state of an input or to set the state of an output.
 * <br/>
 * <code>Channel</code> objects always have a reference to a module object. The abstract class
 * <code>Module</code> implements interfaces that the channel objects use to access periphery
 * hardware. Specific device drivers must therefore implement this module class.
 * <br/><br/>
 * This library also contains a number of implementations of the module class, and therefore
 * of specific device drivers. Some drivers use fieldbusses like CAN or EtherCAT, which in
 * turn require additional classes offering CAN-Bus connectivity, representations of messages
 * or the handling of application layer protocols like CANopen.
 * <br/><br/>
 * This object-oriented device driver framework allows to decouple the individual input and
 * output channels in an industrial control application from the actual hardware used. When
 * the hardware architecture of an application changes, only the configuration of device
 * drivers needs to be adapted, which is usually done in a <code>main()</code> function that
 * is called when the application launches. Other classes, like classes implementing motion
 * control algorithms or classes implementing state machines are not affected by changes of
 * the hardware architecture.
 * <br/><br/>
 * The Industrial Input/Output library also includes a number of other useful classes for the
 * design of industrial control software, like an embedded webserver, which also allows to
 * execute virtual cgi-bin scripts, or classes that allow to implement periodic realtime tasks
 * or nonperiodic background tasks.
 * <br/><br/>
 * See the documentation about the individual classes for more information.
 */
