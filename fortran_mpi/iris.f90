module iris
    use, intrinsic :: iso_c_binding
    implicit none

interface
    integer(C_INT) FUNCTION iris_model_init(iris, modelnb) BIND(C, name="Iris_Model_Init")
        use, intrinsic :: iso_c_binding
        implicit none
        type(C_PTR), value :: iris
        integer(C_INT), value :: modelnb
    end function iris_model_init

    integer FUNCTION firis_model_init(iris, modelnb) BIND(C, name="Fortran_Iris_Model_Init")
        use, intrinsic :: iso_c_binding
        implicit none
        type(C_PTR), value :: iris
        integer(C_INT), value :: modelnb
    end function firis_model_init

    type(C_PTR) FUNCTION iris_new() BIND(C, name="Iris_New")
        use, intrinsic :: iso_c_binding
    end FUNCTION iris_new

    integer(C_INT) FUNCTION iris_get_modelno(iris) BIND(C, name="Fortran_Iris_Get_ModelNo")
        use, intrinsic :: iso_c_binding
        implicit none
        type(C_PTR), value :: iris
    end FUNCTION iris_get_modelno

    integer(C_INT) FUNCTION  iris_grid_def(iris, modelno, &
            GNI, GNJ, GNK, GHI, GHJ, &
            TI0, TI1, TJ0, TJ1, THI, THJ) &
    BIND(C, name="Iris_Grid_Def")
        use, intrinsic :: iso_c_binding
        implicit none
        type(C_PTR), value :: iris
        integer(C_INT), value :: modelno
        integer(C_INT), value :: GNI, GNJ, GNK, GHI, GHJ, TI0, TI1, TJ0, TJ1, THI, THJ
    end function iris_grid_def
end interface

end module

! program fcmain
!   use ISO_C_BINDING
!   implicit none
! 
!   integer(C_INT) :: nargs
!   integer :: i, length, status
!   character(len=4096) :: argument
!   character(len=1), dimension(:), pointer :: arg1
!   type(C_PTR), dimension(:), pointer :: argv
!   type(C_PTR) :: argtab
!   interface
!     function c_main(nargs,argv) result(status) BIND(C,name='MY_C_MAIN')
!     import
!     implicit none
!     integer, intent(IN), value :: nargs
!     type(C_PTR), intent(IN), value :: argv
!     integer :: status
!     end function c_main
!   end interface
! 
!   nargs = command_argument_count()
!   allocate(argv(0:nargs+1))
!   argv = C_NULL_PTR
!   do i=0,nargs
!     call get_command_argument(i,argument,length,status)
!     allocate(arg1(length+1))
!     arg1 = transfer(trim(argument)//achar(0),arg1,length+1)
!     argv(i) = C_LOC(arg1(1))
!   enddo
!   argv(nargs+1) = C_NULL_PTR
!   argtab = C_LOC(argv(0))
!   status = c_main(nargs+1,argtab)
!   stop
! end
