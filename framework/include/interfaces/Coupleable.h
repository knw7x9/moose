//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <unordered_map>
#include "MooseTypes.h"
#include "MooseArray.h"
#include "MooseVariableFE.h"
#include "MooseVariableFV.h"
#include "InputParameters.h"
#include "HasMembers.h"

// Forward declarations
class MooseVariableScalar;
class MooseObject;

namespace libMesh
{
template <typename T>
class DenseVector;
}

/**
 * Interface for objects that needs coupling capabilities
 *
 */
class Coupleable
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   * @param nodal true if we need to couple with nodal values, otherwise false
   * @param is_fv Whether the \p MooseObject is a finite volume object
   */
  Coupleable(const MooseObject * moose_object, bool nodal, bool is_fv = false);

  /**
   * Get the list of coupled variables
   * @return The list of coupled variables
   */
  const std::unordered_map<std::string, std::vector<MooseVariableFieldBase *>> & getCoupledVars()
  {
    return _coupled_vars;
  }

  /**
   * Get the list of all coupled variables
   * @return The list of all coupled variables
   */
  const std::vector<MooseVariableFieldBase *> & getCoupledMooseVars() const
  {
    return _coupled_moose_vars;
  }

  /**
   * Get the list of standard coupled variables
   * @return The list of standard coupled variables
   */
  const std::vector<MooseVariable *> & getCoupledStandardMooseVars() const
  {
    return _coupled_standard_moose_vars;
  }

  /**
   * Get the list of vector coupled variables
   * @return The list of vector coupled variables
   */
  const std::vector<VectorMooseVariable *> & getCoupledVectorMooseVars() const
  {
    return _coupled_vector_moose_vars;
  }

  /**
   * Get the list of array coupled variables
   * @return The list of array coupled variables
   */
  const std::vector<ArrayMooseVariable *> & getCoupledArrayMooseVars() const
  {
    return _coupled_array_moose_vars;
  }

  void addFEVariableCoupleableVectorTag(TagID tag) { _fe_coupleable_vector_tags.insert(tag); }

  void addFEVariableCoupleableMatrixTag(TagID tag) { _fe_coupleable_matrix_tags.insert(tag); }

  std::set<TagID> & getFEVariableCoupleableVectorTags() { return _fe_coupleable_vector_tags; }

  std::set<TagID> & getFEVariableCoupleableMatrixTags() { return _fe_coupleable_matrix_tags; }

protected:
  /**
   * A call-back function provided by the derived object for actions before coupling a variable
   * with functions such as coupledValue.
   */
  virtual void coupledCallback(const std::string & /*var_name*/, bool /*is_old*/) {}

  /**
   * Returns true if a variables has been coupled as name.
   * @param var_name The name the kernel wants to refer to the variable as.
   * @param i By default 0, in general the index to test in a vector of MooseVariable pointers.
   * @return True if a coupled variable has the supplied name
   */
  virtual bool isCoupled(const std::string & var_name, unsigned int i = 0);

  /**
   * Number of coupled components
   * @param var_name Name of the variable
   * @return number of components this variable has (usually 1)
   */
  unsigned int coupledComponents(const std::string & var_name);

  /**
   * Returns the index for a coupled variable by name
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Index of coupled variable, if this is an optionally coupled variable that wasn't
   * provided this will return a unique "invalid" index.
   */
  virtual unsigned int coupled(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns value of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::_u
   */
  virtual const VariableValue & coupledValue(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns value of a coupled variable for use in Automatic Differentiation
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a ADVariableValue for the coupled variable
   */
  const ADVariableValue & adCoupledValue(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns value of a coupled vector variable for use in Automatic Differentiation
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::value
   */
  const ADVectorVariableValue & adCoupledVectorValue(const std::string & var_name,
                                                     unsigned int comp = 0);

  /**
   * Returns value of a coupled variable for a given tag
   * @param var_name Name of coupled variable
   * @param tag vector tag ID
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::_u
   */
  virtual const VariableValue &
  coupledVectorTagValue(const std::string & var_name, TagID tag, unsigned int comp = 0);

  /**
   * Returns value of a coupled variable for a given tag. This couples the diag vector of matrix
   * @param var_name Name of coupled variable
   * @param tag matrix tag ID
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::_u
   */
  virtual const VariableValue &
  coupledMatrixTagValue(const std::string & var_name, TagID tag, unsigned int comp = 0);

  /**
   * Returns value of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue for the coupled vector variable
   * @see VectorKernel::_u
   */
  virtual const VectorVariableValue & coupledVectorValue(const std::string & var_name,
                                                         unsigned int comp = 0);

  /**
   * Returns value of a coupled array variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a ArrayVariableValue for the coupled vector variable
   * @see ArrayKernel::_u
   */
  virtual const ArrayVariableValue & coupledArrayValue(const std::string & var_name,
                                                       unsigned int comp = 0);

  /**
   * Returns a *writable* reference to a coupled variable.  Note: you
   * should not have to use this very often (use coupledValue()
   * instead) but there are situations, such as writing to multiple
   * AuxVariables from a single AuxKernel, where it is required.
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::value
   */
  virtual VariableValue & writableCoupledValue(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns an old value from previous time step  of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the old value of the coupled variable
   * @see Kernel::valueOld
   */
  virtual const VariableValue & coupledValueOld(const std::string & var_name,
                                                unsigned int comp = 0);

  /**
   * Returns an old value from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the older value of the coupled variable
   * @see Kernel::valueOlder
   */
  virtual const VariableValue & coupledValueOlder(const std::string & var_name,
                                                  unsigned int comp = 0);

  /**
   * Returns value of previous Newton iterate of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the older value of the coupled variable
   */
  virtual const VariableValue & coupledValuePreviousNL(const std::string & var_name,
                                                       unsigned int comp = 0);

  /**
   * Returns an old value from previous time step  of a coupled vector variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableValue containing the old value of the coupled variable
   * @see VectorKernel::_u_old
   */
  virtual const VectorVariableValue & coupledVectorValueOld(const std::string & var_name,
                                                            unsigned int comp = 0);

  /**
   * Returns an old value from two time steps previous of a coupled vector variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableValue containing the older value of the coupled variable
   * @see VectorKernel::_u_older
   */
  virtual const VectorVariableValue & coupledVectorValueOlder(const std::string & var_name,
                                                              unsigned int comp = 0);

  /**
   * Returns an old value from previous time step  of a coupled array variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a ArrayVariableValue containing the old value of the coupled variable
   * @see ArrayKernel::_u_old
   */
  virtual const ArrayVariableValue & coupledArrayValueOld(const std::string & var_name,
                                                          unsigned int comp = 0);

  /**
   * Returns an old value from two time steps previous of a coupled array variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a ArrayVariableValue containing the older value of the coupled variable
   * @see ArrayKernel::_u_older
   */
  virtual const ArrayVariableValue & coupledArrayValueOlder(const std::string & var_name,
                                                            unsigned int comp = 0);

  /**
   * Returns gradient of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the gradient of the coupled variable
   * @see Kernel::gradient
   */
  virtual const VariableGradient & coupledGradient(const std::string & var_name,
                                                   unsigned int comp = 0);

  /**
   * Returns gradient of a coupled variable for use in Automatic Differentation
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the gradient of the coupled variable
   * @see Kernel::gradient
   */
  const ADVariableGradient & adCoupledGradient(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns gradient of a coupled vector variable for use in Automatic Differentation
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableGradient containing the gradient of the coupled variable
   * @see Kernel::gradient
   */
  const ADVectorVariableGradient & adCoupledVectorGradient(const std::string & var_name,
                                                           unsigned int comp = 0);

  /**
   * Returns second derivatives of a coupled variable for use in Automatic Differentation
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the second derivatives of the coupled variable
   */
  const ADVariableSecond & adCoupledSecond(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns second derivatives of a coupled vector variable for use in Automatic Differentation
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableSecond containing the second derivatives of the coupled
   * variable
   */
  const ADVectorVariableSecond & adCoupledVectorSecond(const std::string & var_name,
                                                       unsigned int comp = 0);

  /**
   * Returns an old gradient from previous time step of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the old gradient of the coupled variable
   * @see Kernel::gradientOld
   */
  virtual const VariableGradient & coupledGradientOld(const std::string & var_name,
                                                      unsigned int comp = 0);

  /**
   * Returns an old gradient from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the older gradient of the coupled variable
   * @see Kernel::gradientOlder
   */
  virtual const VariableGradient & coupledGradientOlder(const std::string & var_name,
                                                        unsigned int comp = 0);

  /**
   * Returns gradient of a coupled variable for previous Newton iterate
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the gradient of the coupled variable
   */
  virtual const VariableGradient & coupledGradientPreviousNL(const std::string & var_name,
                                                             unsigned int comp = 0);

  /**
   * Time derivative of the gradient of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the time derivative of the gradient of a
   * coupled variable
   */
  virtual const VariableGradient & coupledGradientDot(const std::string & var_name,
                                                      unsigned int comp = 0);

  /**
   * Second time derivative of the gradient of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the time derivative of the gradient of a
   * coupled variable
   */
  virtual const VariableGradient & coupledGradientDotDot(const std::string & var_name,
                                                         unsigned int comp = 0);

  /**
   * Returns gradient of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableGradient containing the gradient of the coupled vector
   * variable
   */
  virtual const VectorVariableGradient & coupledVectorGradient(const std::string & var_name,
                                                               unsigned int comp = 0);

  /**
   * Returns an old gradient from previous time step of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableGradient containing the old gradient of the coupled vector
   * variable
   */
  virtual const VectorVariableGradient & coupledVectorGradientOld(const std::string & var_name,
                                                                  unsigned int comp = 0);

  /**
   * Returns an old gradient from two time steps previous of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableGradient containing the older gradient of the coupled
   * vector variable
   */
  virtual const VectorVariableGradient & coupledVectorGradientOlder(const std::string & var_name,
                                                                    unsigned int comp = 0);

  /**
   * Returns gradient of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a VectorVariableGradient containing the gradient of the coupled array
   * variable
   */
  virtual const ArrayVariableGradient & coupledArrayGradient(const std::string & var_name,
                                                             unsigned int comp = 0);

  /**
   * Returns an old gradient from previous time step of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a VectorVariableGradient containing the old gradient of the coupled array
   * variable
   */
  virtual const ArrayVariableGradient & coupledArrayGradientOld(const std::string & var_name,
                                                                unsigned int comp = 0);

  /**
   * Returns an old gradient from two time steps previous of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableGradient containing the older gradient of the coupled
   * array variable
   */
  virtual const ArrayVariableGradient & coupledArrayGradientOlder(const std::string & var_name,
                                                                  unsigned int comp = 0);

  /**
   * Returns curl of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableCurl containing the curl of the coupled variable
   * @see Kernel::_curl_u
   */
  virtual const VectorVariableCurl & coupledCurl(const std::string & var_name,
                                                 unsigned int comp = 0);

  /**
   * Returns an old curl from previous time step of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableCurl containing the old curl of the coupled variable
   * @see Kernel::_curl_u_old
   */
  virtual const VectorVariableCurl & coupledCurlOld(const std::string & var_name,
                                                    unsigned int comp = 0);

  /**
   * Returns an old curl from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableCurl containing the older curl of the coupled variable
   * @see Kernel::_curl_u_older
   */
  virtual const VectorVariableCurl & coupledCurlOlder(const std::string & var_name,
                                                      unsigned int comp = 0);

  /**
   * Returns second derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the second derivative of the coupled variable
   * @see Kernel::second
   */
  virtual const VariableSecond & coupledSecond(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns an old second derivative from previous time step of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the old second derivative of the coupled
   * variable
   * @see Kernel::secondOld
   */
  virtual const VariableSecond & coupledSecondOld(const std::string & var_name,
                                                  unsigned int comp = 0);

  /**
   * Returns an old second derivative from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the older second derivative of the coupled
   * variable
   * @see Kernel::secondOlder
   */
  virtual const VariableSecond & coupledSecondOlder(const std::string & var_name,
                                                    unsigned int comp = 0);

  /**
   * Returns second derivative of a coupled variable for the previous Newton iterate
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the second derivative of the coupled variable
   */
  virtual const VariableSecond & coupledSecondPreviousNL(const std::string & var_name,
                                                         unsigned int comp = 0);

  /**
   * Time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   */
  virtual const VariableValue & coupledDot(const std::string & var_name, unsigned int comp = 0);

  /**
   * Second time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   */
  virtual const VariableValue & coupledDotDot(const std::string & var_name, unsigned int comp = 0);

  /**
   * Old time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   */
  virtual const VariableValue & coupledDotOld(const std::string & var_name, unsigned int comp = 0);

  /**
   * Old second time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   */
  virtual const VariableValue & coupledDotDotOld(const std::string & var_name,
                                                 unsigned int comp = 0);

  /**
   * Time derivative of a coupled variable for ad simulations
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   * @see Kernel::dot
   */
  const ADVariableValue & adCoupledDot(const std::string & var_name, unsigned int comp = 0);

  /**
   * Time derivative of a vector coupled variable for ad simulations
   * @param var_name Name of vector coupled variable
   * @param comp Component number
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   * @see Kernel::dot
   */
  const ADVectorVariableValue & adCoupledVectorDot(const std::string & var_name,
                                                   unsigned int comp = 0);

  /**
   * Time derivative of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const VectorVariableValue & coupledVectorDot(const std::string & var_name,
                                                       unsigned int comp = 0);

  /**
   * Second time derivative of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const VectorVariableValue & coupledVectorDotDot(const std::string & var_name,
                                                          unsigned int comp = 0);

  /**
   * Old time derivative of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const VectorVariableValue & coupledVectorDotOld(const std::string & var_name,
                                                          unsigned int comp = 0);

  /**
   * Old second time derivative of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const VectorVariableValue & coupledVectorDotDotOld(const std::string & var_name,
                                                             unsigned int comp = 0);

  /**
   * Time derivative of a coupled vector variable with respect to the coefficients
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VariableValue containing the time derivative of the coupled
   * vector variable with respect to the coefficients
   */
  virtual const VariableValue & coupledVectorDotDu(const std::string & var_name,
                                                   unsigned int comp = 0);

  /**
   * Second time derivative of a coupled vector variable with respect to the coefficients
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VariableValue containing the time derivative of the coupled vector
   * variable with respect to the coefficients
   */
  virtual const VariableValue & coupledVectorDotDotDu(const std::string & var_name,
                                                      unsigned int comp = 0);

  /**
   * Time derivative of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const ArrayVariableValue & coupledArrayDot(const std::string & var_name,
                                                     unsigned int comp = 0);

  /**
   * Second time derivative of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const ArrayVariableValue & coupledArrayDotDot(const std::string & var_name,
                                                        unsigned int comp = 0);

  /**
   * Old time derivative of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const ArrayVariableValue & coupledArrayDotOld(const std::string & var_name,
                                                        unsigned int comp = 0);

  /**
   * Old second time derivative of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const ArrayVariableValue & coupledArrayDotDotOld(const std::string & var_name,
                                                           unsigned int comp = 0);

  /**
   * Time derivative of a coupled variable with respect to the coefficients
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   * with respect to the coefficients
   */
  virtual const VariableValue & coupledDotDu(const std::string & var_name, unsigned int comp = 0);

  /**
   * Second time derivative of a coupled variable with respect to the coefficients
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   * with respect to the coefficients
   */
  virtual const VariableValue & coupledDotDotDu(const std::string & var_name,
                                                unsigned int comp = 0);

  /**
   * Returns nodal values of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   */
  template <typename T>
  const T & coupledNodalValue(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns AD nodal values of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   */
  template <typename T>
  const typename Moose::ADType<T>::type & adCoupledNodalValue(const std::string & var_name,
                                                              unsigned int comp = 0);

  /**
   * Returns an old nodal value from previous time step  of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the old value of the coupled variable
   */
  template <typename T>
  const T & coupledNodalValueOld(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns an old nodal value from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the older value of the coupled variable
   */
  template <typename T>
  const T & coupledNodalValueOlder(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns nodal values of a coupled variable for previous Newton iterate
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   */
  template <typename T>
  const T & coupledNodalValuePreviousNL(const std::string & var_name, unsigned int comp = 0);

  /**
   * Nodal values of time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the nodal values of time derivative of the
   * coupled variable
   */
  template <typename T>
  const T & coupledNodalDot(const std::string & var_name, unsigned int comp = 0);

  /**
   * Nodal values of second time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the nodal values of second time derivative of
   * the coupled variable
   */
  virtual const VariableValue & coupledNodalDotDot(const std::string & var_name,
                                                   unsigned int comp = 0);

  /**
   * Nodal values of old time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the nodal values of time derivative of the
   * coupled variable
   */
  virtual const VariableValue & coupledNodalDotOld(const std::string & var_name,
                                                   unsigned int comp = 0);

  /**
   * Nodal values of old second time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the nodal values of second time derivative of
   * the coupled variable
   */
  virtual const VariableValue & coupledNodalDotDotOld(const std::string & var_name,
                                                      unsigned int comp = 0);
  // coupled-dof-values-begin
  /**
   * Returns DoFs in the current solution vector of a coupled variable for the local element
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the DoFs of the coupled variable
   */
  virtual const VariableValue & coupledDofValues(const std::string & var_name,
                                                 unsigned int comp = 0);

  /**
   * Returns DoFs in the old solution vector of a coupled variable for the local element
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the old DoFs of the coupled variable
   */
  virtual const VariableValue & coupledDofValuesOld(const std::string & var_name,
                                                    unsigned int comp = 0);

  /**
   * Returns DoFs in the older solution vector of a coupled variable for the local element
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the older DoFs of the coupled variable
   */
  virtual const VariableValue & coupledDofValuesOlder(const std::string & var_name,
                                                      unsigned int comp = 0);
  // coupled-dof-values-end

  /**
   *  method that returns _zero to RESIDUAL computing objects and _ad_zero to JACOBIAN
   * computing objects
   */
  const ADVariableValue & adZeroValue();

  /**
   *  method that returns _grad_zero to RESIDUAL computing objects and _ad_grad_zero to
   * JACOBIAN computing objects
   */
  const ADVariableGradient & adZeroGradient();

  /**
   * Retrieve a zero second for automatic differentiation
   */
  const ADVariableSecond & adZeroSecond();

protected:
  // Reference to the interface's input parameters
  const InputParameters & _c_parameters;

  /// The name of the object this interface is part of
  const std::string & _c_name;

  // Reference to FEProblemBase
  FEProblemBase & _c_fe_problem;

  /// Coupled vars whose values we provide
  std::unordered_map<std::string, std::vector<MooseVariableFieldBase *>> _coupled_vars;

  /// Vector of all coupled variables
  std::vector<MooseVariableFieldBase *> _coupled_moose_vars;

  /// Vector of standard coupled variables
  std::vector<MooseVariable *> _coupled_standard_moose_vars;

  /// Vector of vector coupled variables
  std::vector<VectorMooseVariable *> _coupled_vector_moose_vars;

  /// Vector of array coupled variables
  std::vector<ArrayMooseVariable *> _coupled_array_moose_vars;

  /// Vector of standard finite volume oupled variables
  std::vector<MooseVariableFV<Real> *> _coupled_standard_fv_moose_vars;

  /// True if we provide coupling to nodal values
  bool _c_nodal;

  /// True if implicit value is required
  bool _c_is_implicit;

  /// Thread ID of the thread using this object
  THREAD_ID _c_tid;

  /// Will hold the default value for optional coupled variables.
  std::unordered_map<std::string, std::vector<std::unique_ptr<VariableValue>>> _default_value;

  /// Will hold the default value for optional coupled variables for automatic differentiation.
  std::unordered_map<std::string, std::unique_ptr<MooseArray<DualReal>>> _ad_default_value;

  /// Will hold the default value for optional vector coupled variables.
  std::unordered_map<std::string, std::unique_ptr<VectorVariableValue>> _default_vector_value;

  /// Will hold the default value for optional array coupled variables.
  std::map<std::string, ArrayVariableValue *> _default_array_value;

  /// Will hold the default value for optional vector coupled variables for automatic differentiation.
  std::unordered_map<std::string, std::unique_ptr<MooseArray<ADRealVectorValue>>>
      _ad_default_vector_value;

  /**
   * This will always be zero because the default values for optionally coupled variables is always
   * constant and this is used for time derivative info
   */
  VariableValue _default_value_zero;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  VariableGradient _default_gradient;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  MooseArray<ADRealVectorValue> _ad_default_gradient;

  /// This will always be zero because the default values for optionally coupled vector variables is always constant
  MooseArray<ADRealTensorValue> _ad_default_vector_gradient;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  VariableSecond _default_second;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  MooseArray<ADRealTensorValue> _ad_default_second;

  /// Zero value of a variable
  const VariableValue & _zero;
  const VariablePhiValue & _phi_zero;
  const MooseArray<DualReal> & _ad_zero;

  /// Zero gradient of a variable
  const VariableGradient & _grad_zero;
  const MooseArray<ADRealVectorValue> & _ad_grad_zero;

  /// Zero gradient of trial function
  const VariablePhiGradient & _grad_phi_zero;

  /// Zero second derivative of a variable
  const VariableSecond & _second_zero;
  const MooseArray<ADRealTensorValue> & _ad_second_zero;
  /// Zero second derivative of a test function
  const VariablePhiSecond & _second_phi_zero;
  /// Zero value of a vector variable
  const VectorVariableValue & _vector_zero;
  /// Zero value of the curl of a vector variable
  const VectorVariableCurl & _vector_curl_zero;

  /**
   * This will always be zero because the default values for optionally coupled variables is always
   * constant and this is used for time derivative info
   */
  VectorVariableValue _default_vector_value_zero;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  VectorVariableGradient _default_vector_gradient;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  VectorVariableCurl _default_vector_curl;

  /**
   * This will always be zero because the default values for optionally coupled variables is always
   * constant and this is used for time derivative info
   */
  ArrayVariableValue _default_array_value_zero;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  ArrayVariableGradient _default_array_gradient;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  ArrayVariableCurl _default_array_curl;

  /**
   * Check that the right kind of variable is being coupled in
   *
   * @param var_name The name of the coupled variable
   */
  bool checkVar(const std::string & var_name, unsigned int comp = 0, unsigned int comp_bound = 0);

private:
  enum class FuncAge
  {
    Curr,
    Old,
    Older,
  };

  enum class VarType
  {
    Ignore,
    Gradient,
    Second,
    GradientDot,
    Dot,
  };

  void checkFuncType(const std::string var_name, VarType t, FuncAge age);

protected:
  /**
   * Deprecated method. Use \p getFieldVar instead
   * Extract pointer to a base coupled field variable. Could be either a finite volume or finite
   * element variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  MooseVariableFieldBase * getFEVar(const std::string & var_name, unsigned int comp);

  /*
   * Extract pointer to a base coupled field variable. Could be either a finite volume or finite
   * element variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  MooseVariableFieldBase * getFieldVar(const std::string & var_name, unsigned int comp);

  /**
   * Helper that that be used to retrieve a variable of arbitrary type \p T
   */
  template <typename T, typename std::enable_if<HasMemberType_OutputShape<T>::value, int>::type = 0>
  T * getVarHelper(const std::string & var_name, unsigned int comp);

  /**
   * Reverse compatibility helper that can be used to retried a variable of type \p
   * MooseVariableFE<T>
   */
  template <typename T,
            typename std::enable_if<!HasMemberType_OutputShape<T>::value, int>::type = 0>
  MooseVariableFE<T> * getVarHelper(const std::string & var_name, unsigned int comp);

  /**
   * Extract pointer to a coupled variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  MooseVariable * getVar(const std::string & var_name, unsigned int comp);

  /**
   * Extract pointer to a coupled vector variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  VectorMooseVariable * getVectorVar(const std::string & var_name, unsigned int comp);

  /**
   * Extract pointer to a coupled array variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  ArrayMooseVariable * getArrayVar(const std::string & var_name, unsigned int comp);

  /**
   * Checks to make sure that the current Executioner has set "_is_transient" when old/older values
   * are coupled in.
   * @param name the name of the variable
   * @param fn_name The name of the function that called this method - used in the error message
   */
  void validateExecutionerType(const std::string & name, const std::string & fn_name) const;

  /// Whether or not this object is a "neighbor" object: ie all of it's coupled values should be neighbor values
  bool _coupleable_neighbor;

public:
  /**
   * Helper method to return (and insert if necessary) the default value for Automatic
   * Differentiation for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default value
   * @return VariableValue * a pointer to the associated VarirableValue.
   */
  ADVariableValue * getADDefaultValue(const std::string & var_name);

  /**
   * Helper method to return (and insert if necessary) the default vector value for Automatic
   * Differentiation for an uncoupled variable.
   * @param var_name the name of the vector variable for which to retrieve a default value
   * @return VariableVectorValue * a pointer to the associated VarirableVectorValue.
   */
  ADVectorVariableValue * getADDefaultVectorValue(const std::string & var_name);

  /**
   * Helper method to return (and insert if necessary) the default gradient for Automatic
   * Differentiation for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default gradient
   * @return VariableGradient * a pointer to the associated VariableGradient.
   */
  ADVariableGradient & getADDefaultGradient();

  /**
   * Helper method to return (and insert if necessary) the default gradient for Automatic
   * Differentiation for an uncoupled vector variable.
   * @param var_name the name of the vector variable for which to retrieve a default gradient
   * @return VariableGradient * a pointer to the associated VectorVariableGradient.
   */
  ADVectorVariableGradient & getADDefaultVectorGradient();

  /**
   * Helper method to return (and insert if necessary) the default second derivatives for Automatic
   * Differentiation for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default second derivative
   * @return VariableSecond * a pointer to the associated VariableSecond.
   */
  ADVariableSecond & getADDefaultSecond();

private:
  /**
   * Helper method to return (and insert if necessary) the default value
   * for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default value
   * @return a pointer to the associated VariableValue.
   */
  VariableValue * getDefaultValue(const std::string & var_name, unsigned int comp);

  /**
   * Helper method to return (and insert if necessary) the default value
   * for an uncoupled vector variable.
   * @param var_name the name of the vector variable for which to retrieve a default value
   * @return a pointer to the associated VectorVariableValue.
   */
  VectorVariableValue * getDefaultVectorValue(const std::string & var_name);

  /**
   * Helper method to return (and insert if necessary) the default value
   * for an uncoupled array variable.
   * @param var_name the name of the vector variable for which to retrieve a default value
   * @return a pointer to the associated VectorVariableValue.
   */
  ArrayVariableValue * getDefaultArrayValue(const std::string & var_name);

  /**
   * Get nodal default value
   */
  template <typename T>
  const T & getDefaultNodalValue(const std::string & var_name, unsigned int comp = 0);

  /// Maximum qps for any element in this system
  unsigned int _coupleable_max_qps;

  /// Unique indices for optionally coupled vars that weren't provided
  std::unordered_map<std::string, std::vector<unsigned int>> _optional_var_index;

  /// Scalar variables coupled into this object (for error checking)
  std::unordered_map<std::string, std::vector<MooseVariableScalar *>> _c_coupled_scalar_vars;

  std::set<TagID> _fe_coupleable_vector_tags;

  std::set<TagID> _fe_coupleable_matrix_tags;

  /// Whether the MooseObject is a finite volume object
  const bool _is_fv;

private:
  const MooseObject * _obj;
};

template <typename T, typename std::enable_if<HasMemberType_OutputShape<T>::value, int>::type>
T *
Coupleable::getVarHelper(const std::string & var_name, unsigned int comp)
{
  if (!checkVar(var_name, comp, 0))
    return nullptr;

  if (auto * coupled_var = dynamic_cast<T *>(_coupled_vars[var_name][comp]))
    return coupled_var;
  else
  {
    for (auto & var : _coupled_standard_moose_vars)
      if (var->name() == var_name)
        mooseError("The named variable is a standard variable, try a "
                   "'coupled[Value/Gradient/Dot/etc]...' function instead");
    for (auto & var : _coupled_vector_moose_vars)
      if (var->name() == var_name)
        mooseError("The named variable is a vector variable, try a "
                   "'coupledVector[Value/Gradient/Dot/etc]...' function instead");
    for (auto & var : _coupled_array_moose_vars)
      if (var->name() == var_name)
        mooseError("The named variable is an array variable, try a "
                   "'coupledArray[Value/Gradient/Dot/etc]...' function instead");
    mooseError(
        "Variable '", var_name, "' is of a different C++ type than you tried to fetch it as.");
  }
}

template <typename T, typename std::enable_if<!HasMemberType_OutputShape<T>::value, int>::type>
MooseVariableFE<T> *
Coupleable::getVarHelper(const std::string & var_name, unsigned int comp)
{
  return getVarHelper<MooseVariableFE<T>>(var_name, comp);
}
