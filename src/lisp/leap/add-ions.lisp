(in-package :leap.add-ions)

(defun add-ions (mol ion1 ion1-number &optional ion2 ion2-number)
  (let* ((force-field (leap.core::merged-force-field))
         (nonbond-db (chem:get-nonbond-db force-field))
         (ion1-type-index (chem:find-type-index nonbond-db ion1))
         (ion1-ffnonbond (chem:get-ffnonbond-using-type-index nonbond-db ion1-type-index))
         (ion1-size (chem:get-radius-angstroms ion1-ffnonbond))
         (ion1-topology (leap.core:lookup-variable ion1))
         (ion1-residue (chem:build-residue ion1-topology))
         (ion1-atom (chem:content-at ion1-residue 0))
         (ion1-mol (chem:make-molecule))
         (ion1-agg (chem:make-aggregate))
         (target-charge 0)
         (ion-min-size 0.0)
         (oct-shell 1)
         (grid-space 1.0)
         (shell-extent 4.0)
         (at-octree 1)
         (dielectric 1)
         (ion2-size 0.0) 
         (ion2-type-index 0)
         ion2-ffnonbond)
    (chem:add-matter ion1-mol ion1-residue)
    (chem:add-matter ion1-agg ion1-mol)
    (if (and ion2 (= 0 ion2-number))
        (error "'0' is not allowed as the value for the second ion."))
    ;;Consider target unit's charge
    (chem:map-atoms
     nil
     (lambda (r)
       (setf target-charge (+ (chem:get-charge r) target-charge)))
     mol)
    (if (and (= 0 target-charge)
             (= 0 ion1-number))
        (error "Can't neutralize.")
        (format t "Total charge ~f~%" target-charge))
    ;;Consider neutralizetion    
    (if (= ion1-number 0)
        (progn
          (if (or (and (< (chem:get-charge ion1-atom) 0)
                       (< target-charge 0))
                  (and (> (chem:get-charge ion1-atom) 0)
                       (> target-charge 0)))
              (error "1st ion & target are same charge"))
          ;;Get the nearest integer number of ions that we need to add to get as close as possible to neutral.
          (setf ion1-number (round (/ (abs target-charge) (abs (chem:get-charge ion1-atom)))))
          (if ion2
              (error "Neutralization - can't do 2nd ion."))
          (format t "~d ~a ions required to neutraize. ~%" ion1-number (chem:get-name ion1-atom))))
    ;;Consider ion sizes and postions
    (if ion2
        (progn
          (setf ion2-type-index (chem:find-type-index nonbond-db ion2))
          (setf ion2-ffnonbond (chem:get-ffnonbond-using-type-index nonbond-db ion2-type-index))
          (setf ion2-size (chem:get-radius-angstroms ion2-ffnonbond))
          (setf ion-min-size (min ion1-size ion2-size)))
        (setf ion-min-size ion1-size))
    (format t "Adding ~d counter ions to ~a using 1A grid. ~%"
            (if ion2 (+ ion1-number ion2-number) ion1-number) (chem:get-name mol))
    (if ion2
        (if (> (+ ion1-number ion2-number) 5)
            (setf ion-min-size (if (> ion1-size ion2-size)
                                   (* ion1-size
                                      (if (> (exp (/ (log (+ ion1-number ion2-number 1.0)) 3.0)) 1.0)
                                          (exp (/ (log (+ ion1-number ion2-number 1.0)) 3.0))
                                          1.0))
                                   (* ion2-size
                                      (if (> (exp (/ (log (+ ion1-number ion2-number 1.0)) 3.0)) 1.0)
                                          (exp (/ (log (+ ion1-number ion2-number 1.0)) 3.0))
                                          1.0)))))
        (if (> ion1-number 5)
            (setf ion-min-size (* ion1-size
                                  (if (> (exp (/ (log (+ ion1-number 1.0)) 3.0)) 1.0)
                                      (exp (/ (log (+ ion1-number 1.0)) 3.0))
                                      1.0)))))
    
    ;;Build grid and calc potential on it
    (let ((oct-tree (core:make-cxx-object 'chem:octree))
          (ion2-mol (chem:make-molecule))
          (ion2-agg (chem:make-aggregate))
          ion2-topology ion2-residue ion2-atom)
      (if ion2
          (progn 
            (setf ion2-topology (leap.core:lookup-variable ion2)
                  ion2-residue (chem:build-residue ion2-topology)
                  ion2-atom (chem:content-at ion2-residue 0))
            (chem:add-matter ion2-mol ion2-residue)
            (chem:add-matter ion2-agg ion2-mol)))
      (chem:oct-oct-tree-create oct-tree mol oct-shell grid-space ion-min-size shell-extent 0 t)
      (multiple-value-bind (min-charge-point max-charge-point)
          (chem:oct-tree-init-charges oct-tree at-octree dielectric ion1-size)
        (loop 
           for new-point = (if (< (chem:get-charge ion1-atom) 0) max-charge-point min-charge-point)
           for ion1-copy = (chem:matter-copy ion1-agg)
           for ion1-transform = (geom:make-m4-translate new-point)
           while (if ion2
                     (or (> ion1-number 0)
                            (> ion2-number 0))
                     (> ion1-number 0))
           do (chem:apply-transform-to-atoms ion1-copy ion1-transform)
           do (chem:add-matter mol ion1-copy)
           do (format t "Placed ~a in ~a at ~a ~a ~a~%" (chem:get-name ion1-atom) 
                      (chem:get-name mol)
                      (geom:vx new-point)
                      (geom:vy new-point)
                      (geom:vz new-point))
           do (chem:oct-tree-delete-sphere oct-tree new-point (if ion2
                                                                  (+ ion1-size ion2-size)
                                                                  (+ ion1-size ion1-size)))
           do (multiple-value-bind (min-new-charge-point max-new-charge-point)
                  (chem:oct-tree-update-charge oct-tree new-point (chem:get-charge ion1-atom)
                                               (if ion2 ion2-size ion1-size))
                (setf min-charge-point min-new-charge-point)
                (setf max-charge-point max-new-charge-point))
           do (decf ion1-number)
           do (if ion2
                  (progn 
                    (if (< (chem:get-charge ion2-atom) 0)
                        (setf new-point max-charge-point)
                        (setf new-point min-charge-point))
                    (let ((ion2-copy (chem:matter-copy ion2-agg))
                          (ion2-transform (geom:make-m4-translate new-point)))
                      (chem:apply-transform-to-atoms ion2-copy ion2-transform)
                      (chem:add-matter mol ion2-copy)
                      (format t "Placed ~a in ~a at ~a ~a ~a~%" (chem:get-name ion2-atom) 
                              (chem:get-name mol)
                              (geom:vx new-point)
                              (geom:vy new-point)
                              (geom:vz new-point))
                      (chem:oct-tree-delete-sphere oct-tree new-point (if ion1
                                                                          (+ ion2-size ion1-size)
                                                                          (+ ion2-size ion2-size)))
                      (multiple-value-bind (min-new-charge-point max-new-charge-point)
                          (chem:oct-tree-update-charge oct-tree new-point (chem:get-charge ion2-atom)
                                                       (if ion1 ion1-size ion2-size))
                        (setf min-charge-point min-new-charge-point)
                        (setf max-charge-point max-new-charge-point)))
                    (decf ion2-number)))
             ) ;;loop end
        ))
    
    )
  )

